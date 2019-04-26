/*************************************************************************

Test 1:

Tests having multiple (2) periodic processes running together with one process that doesn't have a deadline

Expected behavior:

Red LED should blink twice as pNRT1 runs until pRT1 is ready. Then pRT1 should preempt the red LED and we should see 3 blinks of
blue LED. Then, pRT2 should run and we should see 12 blinks of the yellow. Then, while both pRT1 and
pRT2 are not ready again yet we should see 2 more blinks of red LED so that pNRT1 can finish. Then, once we
hit 16 seconds nRT1 runs and then subsequently pNRT2 runs, and repeats this every 16 seconds.

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


/*-------------------
 * Real-time processes
 *-------------------*/

//blinks blue LED 3 times
void pRT1(void) {
	LED_Off();
	int i;
	for (i=0; i<3;i++){
	LEDBlue_On();
	shortDelay();
	LEDBlue_Toggle();
	shortDelay();
	}
}
//blinks yellow 12 times
void pRT2(void) {
	LED_Off();
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
		if (process_rt_periodic(pRT1, RT_STACK, &t_1sec, &t_5sec, &t_16sec) < 0) { return -1; }
    if (process_rt_periodic(pRT2, RT_STACK, &t_1sec, &t_20sec, &t_16sec) < 0) { return -1; }  
   
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
