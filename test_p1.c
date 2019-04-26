/*************************************************************************

Test 1:

Tests having one periodic process

Expected behavior:

Blinks blue LED 3 times every 5 seconds.

 ************************************************************************/
 
#include "utils.h"
#include "3140_concur.h"
#include "realtime.h"

/*--------------------------*/
/* Parameters for test case */
/*--------------------------*/


 
/* Stack space for processes */
#define NRT_STACK 30
#define RT_STACK  30
 


/*--------------------------------------*/
/* Time structs for real-time processes */
/*--------------------------------------*/

/* Constants used for 'work' and 'deadline's */
realtime_t t_0sec = {0, 0};
realtime_t t_1msec = {0, 1};
realtime_t t_1sec = {1, 0};
realtime_t t_2sec = {2, 0};
realtime_t t_5sec = {5, 0};
realtime_t t_10sec = {10, 0};
realtime_t t_20sec = {20, 0};

/* Process start time */
realtime_t t_pRT1 = {1, 0};

 
/*------------------*/
/* Helper functions */
/*------------------*/
void shortDelay(){delay();}
void mediumDelay() {delay(); delay();}

/*-------------------
 * Periodic process
 *-------------------*/

//blinks blue LED 3 times
void pRT1(void) {
	int i;
	for (i=0; i<3;i++){
	LEDBlue_On();
	shortDelay();
	LEDBlue_Toggle();
	shortDelay();
	}
}


/*--------------------------------------------*/
/* Main function - start concurrent execution */
/*--------------------------------------------*/
int main(void) {	
	 
	 LED_Initialize();

    /* Create processes */ 
    if (process_rt_periodic(pRT1, RT_STACK, &t_0sec, &t_2sec, &t_5sec) < 0) { return -1; } 
   
    /* Launch concurrent execution */
	 process_start();

   LED_Off();
   while(process_deadline_miss>0) {
		 LEDGreen_On();
		 shortDelay();
		 LED_Off();
		 shortDelay();
		 process_deadline_miss--;
	}
	
	/* Hang out in infinite loop (so we can inspect variables if we want) */ 
	while (1);
	return 0;
}
