/*************************************************************************

Test case 2:

Checks the case where there are multiple processes without deadlines, and makes sure that these non-deadline
processes run concurrently, and checks that if all tasks with deadlines meet their deadline then we have
no deadline misses.

Expected behavior: Begin running pNRT1 and pNRT2 concurrently, resulting in purple blinks. This should be preempted
by pRT1 once we are past its start time. pRT1 should run until it completes, resulting in 3 blue blinks, and then
after this pNRT1 and pNRT2 should finish the remainder of their concurrent blink (purple). We should not see any
green blinks at the end because pRT1 should easily meet its deadline.

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
realtime_t t_1msec = {0, 1};
realtime_t t_10sec = {10, 0};
realtime_t t_20sec = {20, 0};

/* Process start time */
realtime_t t_pRT1 = {1, 0};

 
/*------------------*/
/* Helper functions */
/*------------------*/
void shortDelay(){delay();}
void mediumDelay() {delay(); delay();}



/*----------------------------------------------------
 * Non real-time process 1
 *   Blinks red LED 4 times. Should execute concurrently with other non real-time processes
 *----------------------------------------------------*/
 
void pNRT1(void) {
	int i;
	for (i=0; i<4;i++){
	LEDRed_Toggle();
	shortDelay();
	LEDRed_Toggle();
	shortDelay();
	}
	
}

/*----------------------------------------------------
 * Non real-time process 1
 *   Blinks blue LED 4 times. Should execute concurrently with other non real-time processes
 *----------------------------------------------------*/

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
 * Real-time process
	* blinks blue LED 3 times
 *-------------------*/

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
    if (process_create(pNRT1, NRT_STACK) < 0) { return -1; }
		if (process_create(pNRT2, NRT_STACK) < 0) { return -1; }
    if (process_rt_create(pRT1, RT_STACK, &t_pRT1, &t_20sec) < 0) { return -1; }    
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
