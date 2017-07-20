#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

#include "card_reader_adaptor.h"

int beep_style = NO_BEEP;

void* beep_loop(void*)
{

	while(1)
	{
		if(beep_style==SUCCESS_BEEP)
		{
			pinMode (PHYS_PIN_NO_BEEP, OUTPUT);
            digitalWrite(PHYS_PIN_NO_BEEP,HIGH);
			usleep(SUCCESS_BEEP_DURATION_MACRO_SECOND);
			digitalWrite(PHYS_PIN_NO_BEEP,LOW);
			beep_style=NO_BEEP;
		}
		else if(beep_style==FAIL_BEEP)
		{
			pinMode (PHYS_PIN_NO_BEEP, OUTPUT);
            digitalWrite(PHYS_PIN_NO_BEEP,HIGH);
			usleep(FAIL_BEEP_DURATION_MACRO_SECOND);
			digitalWrite(PHYS_PIN_NO_BEEP,LOW);

			usleep(GRANULARITY_MACRO_SECOND);

			pinMode (PHYS_PIN_NO_BEEP, OUTPUT);
            digitalWrite(PHYS_PIN_NO_BEEP,HIGH);
			usleep(FAIL_BEEP_DURATION_MACRO_SECOND);
			digitalWrite(PHYS_PIN_NO_BEEP,LOW);

			beep_style=NO_BEEP;

		}

		usleep(GRANULARITY_MACRO_SECOND);

	}
}
