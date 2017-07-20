#define GRANULARITY_MACRO_SECOND	100000
#define OPEN_DURATION_SECOND	4
#define FAIL_BEEP_DURATION_MACRO_SECOND		100000
#define SUCCESS_BEEP_DURATION_MACRO_SECOND	300000


extern int PHYS_PIN_NO_D0;
extern int PHYS_PIN_NO_D1;
extern int PHYS_PIN_NO_OPEN;
extern int PHYS_PIN_NO_BTN;
extern int PHYS_PIN_NO_BEEP;
extern int is_remote;
extern char remote_url[256];

//declarations for open_door.cpp
void *open_door_loop(void*);
extern bool b_open_door;
extern bool b_keep_open;
typedef struct timeval TimeVal;
extern TimeVal t_stop_opening; 

//declarations for beep.cpp
#define NO_BEEP		0
#define SUCCESS_BEEP	1
#define FAIL_BEEP	2
extern int beep_style;
void* beep_loop(void*);
