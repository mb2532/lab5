/*************************************************************************

Test 1:

Tests having multiple (2) processes with deadlines scheduled together, and also checks that we propperly
record missed deadlines. In addition, it checks that if the only program queued is not ready to start yet
then we correctly wait until it is ready and then execute it.

Expected behavior:

Begin running pNRT1 (blinking red LED). Quickly preempt with pRT1 (blinking blue 3 times). Then finish
pNRT1 (blinking red) becuase pRT2 is not ready yet. Finish pNRT1, and then wait for a bit until pRT2 is ready and
execute pRT2 (blinking yellow) until it completes. Then we should observe one blink of the green LED to
indicate that pRT1 missed its deadline.

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
realtime_t t_7sec = {7, 0};
realtime_t t_8sec = {8, 0};
realtime_t t_10sec = {10, 0};
realtime_t t_16sec = {16, 0};
realtime_t t_20sec = {20, 0};

/* Process start time */
realtime_t t_pRT1 = {1, 0};

 
/*------------------*/
/* Helper functions */
/*------------------*/
void shortDelay(){delay();}
void mediumDelay() {delay(); delay();}



/*----------------------------------------------------
 * Non real-time processes
 *----------------------------------------------------*/
 
//blinks red LED 4 times
void pNRT1(void) {
	int i;
	for (i=0; i<4;i++){
	LEDRed_Toggle();
	shortDelay();
	LEDRed_Toggle();
	shortDelay();
	}
	
}

//blinks blue LED 4 times
void pNRT2(void) {
	int i;
	for (i=0; i<4;i++){
	LEDBlue_Toggle();
	shortDelay();
	LEDBlue_Toggle();
	shortDelay();
	}
	
}



/*-------------------
 * Real-time processes
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
//blinks yellow 3 times
void pRT2(void) {
	int i;
	for (i=0; i<12;i++){
	LEDGreen_Toggle();
	LEDRed_Toggle();
	shortDelay();
	LEDGreen_Toggle();
	LEDRed_Toggle();
	shortDelay();
	}
}


/*--------------------------------------------*/
/* Main function - start concurrent execution */
/*--------------------------------------------*/
int main(void) {	
	 
	 LED_Initialize();

    /* Create processes */ 
    if (process_create(pNRT1, NRT_STACK) < 0) { return -1; }
		//if (process_create(pNRT2, NRT_STACK) < 0) { return -1; }
		if (process_rt_periodic(pRT1, RT_STACK, &t_0sec, &t_5sec, &t_16sec) < 0) { return -1; }
    if (process_rt_periodic(pRT2, RT_STACK, &t_0sec, &t_20sec, &t_16sec) < 0) { return -1; }  
   
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
