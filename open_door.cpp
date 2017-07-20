#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <curl/curl.h>

#include "card_reader_adaptor.h"
bool b_open_door;
bool b_keep_open;
TimeVal t_stop_opening;

const int open_loop_total_count = OPEN_DURATION_SECOND*1000000/GRANULARITY_MACRO_SECOND;

int remote_open_door(){

	 CURL* openurl;
         openurl=curl_easy_init();
         if(openurl){
                 curl_easy_setopt(openurl, CURLOPT_URL,remote_url);
                 curl_easy_setopt(openurl, CURLOPT_HTTPGET, 1L);
                 curl_easy_setopt(openurl, CURLOPT_NOBODY, 1L);
                 curl_easy_perform(openurl);
         }
         curl_easy_cleanup(openurl);

}



void *open_door_loop(void*)
{
	int open_loop_count=0;
	while(1)
	{
		if(b_open_door)
		{
			b_open_door=false;
			if(!is_remote)
			{
				pinMode (PHYS_PIN_NO_OPEN, OUTPUT);
		            	digitalWrite(PHYS_PIN_NO_OPEN,HIGH);
				open_loop_count=open_loop_total_count;
			}
			else
			{
				remote_open_door();	
			}
		}

		usleep(GRANULARITY_MACRO_SECOND);

		if(open_loop_count==0 && b_keep_open==false)
		{
			digitalWrite(PHYS_PIN_NO_OPEN,LOW);
		}
		if(open_loop_count>-1)
			open_loop_count--;
		
		if(b_keep_open)
		{
			TimeVal t_now;
	        	gettimeofday(&t_now, NULL);

	        	if(t_now.tv_sec>t_stop_opening.tv_sec)
	        	{
	        		b_keep_open=false;
	        		digitalWrite(PHYS_PIN_NO_OPEN,LOW);
			}
		}		
	
	}

}
