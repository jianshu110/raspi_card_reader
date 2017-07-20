#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>
#include <curl/curl.h>
#include <microhttpd.h>

#include "card_reader_adaptor.h"

#define TYPE "GAC-300"
int PHYS_PIN_NO_D0=16;
int PHYS_PIN_NO_D1=18;
int PHYS_PIN_NO_OPEN=7;
int PHYS_PIN_NO_BTN=11;
int PHYS_PIN_NO_BEEP=12;
int PHYS_PIN_NO_REST=22;
int is_remote=0;

char type_buf[256];
char remote_url[256];

void init_pin_cfg()
{

	char cfg_buf[256];
	FILE * cfg_file;
	cfg_file = fopen("/etc/card_reader_adaptor/pin.cfg","r");
	if(cfg_file)
	{
		fgets(cfg_buf,256,cfg_file);
		strncpy(type_buf,cfg_buf,7);
		
		if(!(strcmp(type_buf,TYPE)))
		{	
			
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_D0=atoi(cfg_buf);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_D1=atoi(cfg_buf);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_OPEN=atoi(cfg_buf);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_BTN=atoi(cfg_buf);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_BEEP=atoi(cfg_buf);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_REST=atoi(cfg_buf);
			if(!feof(cfg_file))
			{
				if ((NULL!=fgets(cfg_buf,256,cfg_file)))
				{
					is_remote=1;
					strcpy(remote_url,cfg_buf);
					remote_url[strlen(remote_url)-1]=0;
				}
			}
			
		}
		else 
		{
			
			fseek(cfg_file, 0L, SEEK_SET);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_D0=atoi(cfg_buf);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_D1=atoi(cfg_buf);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_OPEN=atoi(cfg_buf);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_BTN=atoi(cfg_buf);
			fgets(cfg_buf,256,cfg_file);
			PHYS_PIN_NO_BEEP=atoi(cfg_buf);
			if(!feof(cfg_file))
			{
				if ((NULL!=fgets(cfg_buf,256,cfg_file)))
				{
					is_remote=1;
					strcpy(remote_url,cfg_buf);
					remote_url[strlen(remote_url)-1]=0;
				}
			}
			
		}
		fclose(cfg_file);
	}
}


#define BIT_NUM 32
time_t t_last;
time_t t_now;
TimeVal	t_last_micro,t_now_micro; 

volatile int eventCounter = BIT_NUM;
volatile unsigned int data;
volatile int bswitch=0;

void myInterrupt_d1(void) {
	
	static bool first = true;
	if(first)
	{
		first=false;
		return;
	}
	printf("d1:%d\n",eventCounter);
        int gap,gap_micro;
        t_now=time(NULL);
        gettimeofday(&t_now_micro, NULL);

        gap = (int)(t_now-t_last);
        gap_micro = t_now_micro.tv_usec-t_last_micro.tv_usec;
        t_last_micro.tv_usec=t_now_micro.tv_usec;
        t_last=t_now;

        if((gap*1000000.0+gap_micro)>50000.0)
                eventCounter=BIT_NUM;
        if(eventCounter>=0)
                data |= (1<<eventCounter);

	if(eventCounter==0)
                bswitch=1;
        eventCounter--;
}

void myInterrupt_d0(void) {
	static bool first = true;
        if(first)
        {
                first=false;
                return;
        }

	printf("d0:%d\n",eventCounter);
        int gap,gap_micro;
        t_now=time(NULL);
        gettimeofday(&t_now_micro, NULL);

        gap = (int)(t_now-t_last);
        gap_micro = t_now_micro.tv_usec-t_last_micro.tv_usec;
        t_last_micro.tv_usec=t_now_micro.tv_usec;
        t_last=t_now;

        if((gap*1000000.0+gap_micro)>50000.0)
                eventCounter=BIT_NUM;
        if(eventCounter>=0)
                data&=(~(1<<eventCounter));

        if(eventCounter==0)
                bswitch=1;
        eventCounter--;
}

int init_pins()
{
	if ( wiringPiISR (PHYS_PIN_NO_D0, INT_EDGE_FALLING, myInterrupt_d0) < 0 ) {
		return 1;
	}
	if ( wiringPiISR (PHYS_PIN_NO_D1, INT_EDGE_FALLING, myInterrupt_d1) < 0 ) {
		return 1;
	}

	pullUpDnControl(PHYS_PIN_NO_D0, PUD_UP);
	pullUpDnControl(PHYS_PIN_NO_D1, PUD_UP);

	pinMode (PHYS_PIN_NO_OPEN, OUTPUT);
	digitalWrite (PHYS_PIN_NO_OPEN, LOW);

	pinMode(PHYS_PIN_NO_BTN,INPUT);
	pullUpDnControl(PHYS_PIN_NO_BTN,PUD_UP);
	if(!(strcmp(type_buf,TYPE)))
	{
		
		pinMode(PHYS_PIN_NO_REST,INPUT);
		pullUpDnControl(PHYS_PIN_NO_REST,PUD_DOWN);
	}
	return 0;
}


int init_wiring_pi()
{
        if (wiringPiSetupPhys()<0)
                return 1;
	init_pins();

        return 0;
}



int initialize()
{
	init_pin_cfg();
	if(init_wiring_pi()!=0)
		return 1;
	
	t_last=time(NULL);	
	return 0;
}

const char * str_ret_default = "card reader adaptor api";

const char * str_ret_open_door = "card reader adaptor api. door open for %d seconds.";
const char * str_url_open_door = "/doors/opendoor/0";
const char * str_query_str_duration = "duration";

const char * str_ret_shut_door = "card reader adaptor api. door shut";
const char * str_url_shut_door = "/doors/shutdoor/0";

const char * str_ret_refuse = "card reader adaptor api. refuse open";
const char * str_url_refuse = "/doors/refuse/0";

int answer_to_connection (void *cls, struct MHD_Connection *connection,
                          const char *url,
                          const char *method, const char *version,
                          const char *upload_data,
                          size_t *upload_data_size, void **con_cls)
{

const char *val;
  char * ret_info;
  char * temp = NULL;
  enum MHD_ResponseMemoryMode mode;
  struct MHD_Response *response;
  int ret;

  ret_info=const_cast<char *>(str_ret_default);
    mode=MHD_RESPMEM_PERSISTENT;

  if (0 == strcmp(url, str_url_open_door) )
  {
    int duration = 4 ;
    
    val = MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, str_query_str_duration);

    if(val!=NULL && strlen(val)>0)
    {
        duration=atoi(val);
        if(duration>4)
        {
          TimeVal t_now;
          gettimeofday(&t_now, NULL);
          t_stop_opening.tv_sec=t_now.tv_sec+duration;
          b_keep_open = true; 
        }
    }

    b_open_door=true;
    beep_style = SUCCESS_BEEP;

    temp = (char*)malloc (snprintf (NULL, 0, str_ret_open_door, str_query_str_duration, duration) + 1);
    if (temp == NULL)
    {
      ret_info= const_cast<char *>(str_ret_open_door);
      
    }
    else
    {
      sprintf (temp, str_ret_open_door, duration);
      ret_info=temp;
      mode = MHD_RESPMEM_MUST_FREE;
    }
  }
  else if(0 == strcmp(url, str_url_refuse))
  {
    beep_style = FAIL_BEEP; 
    ret_info = const_cast<char *>(str_ret_refuse);
    
  }
  else if(0 == strcmp(url, str_url_shut_door))
  {
	gettimeofday(&t_stop_opening,NULL);
    ret_info = const_cast<char *>(str_ret_shut_door);
    
  }



  response = MHD_create_response_from_buffer (strlen (ret_info), ret_info,
                                              mode);
  if (response == NULL)
  {      
    if(temp != NULL)
        free(temp);
    return MHD_NO;
  }
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;

}

#define PORT 8888

int query_privilege(unsigned int id_no,int index){

        CURL* geturl;
        int status_code;

        char str[256];
	char str_req_url[256]="http://127.0.0.1:10080/api/cards/%u/actions/scan/%d";
        sprintf(str,str_req_url,id_no,index);
        struct curl_slist *list = NULL;
	printf(str);
	printf("\n");

        geturl = curl_easy_init();
        if(geturl){

                list = curl_slist_append(list, "Content-Type: application/json");
                curl_easy_setopt(geturl, CURLOPT_HTTPHEADER, list);
                curl_easy_setopt(geturl, CURLOPT_URL,str);
                curl_easy_setopt(geturl, CURLOPT_HTTPPOST, 1L);
                curl_easy_setopt(geturl, CURLOPT_TIMEOUT, 2L);
                str[0]=0;
                curl_easy_setopt(geturl, CURLOPT_POSTFIELDS,str);
                //curl_easy_setopt(geturl, CURLOPT_NOBODY, 1);
                curl_easy_perform(geturl);
                curl_easy_getinfo (geturl, CURLINFO_RESPONSE_CODE, &status_code);

                curl_easy_cleanup(geturl);

        }
        return status_code;
}
/*check_shutdown*/
void *soft_power_off(void*)
{
	int cont=0;
	while(1)
	{
		if(!(strcmp(type_buf,TYPE)))
		{	
			
			if(digitalRead(PHYS_PIN_NO_REST)==1)
			{	
				sleep(1);
				for(int i=0;i<3;i++)
				{
					if(digitalRead(PHYS_PIN_NO_REST)==1)
					{
						cont++;
						sleep(1);
					}
				}
				if((digitalRead(PHYS_PIN_NO_REST)==1)||(cont==3))
	
					system("sudo init 0");
			}
			
			usleep(GRANULARITY_MACRO_SECOND);
		}
		else 
			usleep(GRANULARITY_MACRO_SECOND);
	}
}

int main(int argc, char** argv) {

	b_open_door=false;
	b_keep_open=false;
	pthread_t thread_open_door;
	pthread_t thread_beep;
	pthread_t thread_shutdown;
	int iret_th_open_door=pthread_create( &thread_open_door, NULL, open_door_loop, (void*) NULL );
	int iret_th_beep     =pthread_create( &thread_beep     , NULL,      beep_loop, (void*) NULL );
	int iret_th_shutdown =pthread_create( &thread_shutdown , NULL, soft_power_off, (void*) NULL );
	if(iret_th_open_door)
     	{
        	fprintf(stderr,"Error - pthread_create() return code: %d\n",iret_th_open_door);
         	exit(EXIT_FAILURE);
     	}
	if(iret_th_beep)
        {
                fprintf(stderr,"Error - pthread_create() return code: %d\n",iret_th_beep);
                exit(EXIT_FAILURE);
        }


	 struct MHD_Daemon *daemon;

        daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                             &answer_to_connection, NULL, MHD_OPTION_END);
        if (NULL == daemon)
                return 1;
	if(initialize()!=0)
		return 1;
	FILE * log_ptr;
	int btn_durations=0;
	while(true)
	{
		if(bswitch)
		{
			log_ptr=fopen("/var/log/card_records.log","a+");
			int status=query_privilege(data&0xffffffff,0);
			if((status/100==4)||((status/100==5)))
			{
				beep_style = FAIL_BEEP;
				fprintf(log_ptr,"%u %u %u EXCEPTION\n",data,time(NULL),status);	
			}
			else
				fprintf(log_ptr,"%u %u \n",data,time(NULL));	

			fflush(log_ptr);
			fclose(log_ptr);
			bswitch=0;
		}

		btn_durations=btn_durations<<1;

		if(digitalRead(PHYS_PIN_NO_BTN)==0)
	        {
                	btn_durations=btn_durations|0x01;
       		}
		int temp=0;
        	temp += ((btn_durations)&0x01);
        	temp += ((btn_durations>>1)&0x01);
        	temp += ((btn_durations>>2)&0x01);
        	//temp += ((btn_durations>>3)&0x01);
        	//temp += ((btn_durations>>4)&0x01);
        	if(temp>1)
        	{
			    b_open_door=true;
        	}
		usleep(100000);
	}
	return 0;
}

