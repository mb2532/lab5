#include "3140_concur.h"
#include <stdlib.h>
#include <fsl_device_registers.h>
#include "realtime.h"

struct process_state {
	unsigned int *sp;
	unsigned int *og_sp;
	int stack_size;
	process_t *next;
	realtime_t *start;
	realtime_t *deadline;
	realtime_t *period;
	realtime_t *relative_deadline;
	void (*pc)(void);
};
unsigned int start_time;
process_t *schedule_queue = NULL;
process_t *process_queue = NULL;
process_t *current_process = NULL;

realtime_t current_time;
int process_deadline_met = 0;
int process_deadline_miss = 0;

//reinitialize stack for a periodic process
unsigned int * process_stack_reinit (void (*f)(void), int n, unsigned int *ogsp)
{
    process_stack_free(current_process -> og_sp, current_process -> stack_size);
	  return process_stack_init(f, n);
	
}

unsigned int get_time(realtime_t *time) {
	unsigned int secs = time -> sec;
	unsigned int millisecs = time -> msec;
	return secs*1000 + millisecs;
}

/* removes the first element of process_queue*/
void dequeue() {
	if (process_queue == NULL) {
	}
	else {
		process_queue = process_queue -> next;
	}
}

/* removes the first element of schedule_queue that is ready */
process_t* priority_dequeue() {
	if (schedule_queue == NULL) {
		return NULL;
	}
	else if (get_time(schedule_queue->start) <= current_time.sec*1000 + current_time.msec) {
		process_t *to_remove = schedule_queue;
		schedule_queue = schedule_queue->next;
		return to_remove;
	}
	else {
		process_t *temp = schedule_queue;
		while(temp->next != NULL && get_time(temp->next->start) > current_time.sec*1000 + current_time.msec) {
			temp = temp->next;
			//time = temp->next->start->sec*1000 + temp->next->start->msec;
		}
		if (temp -> next == NULL) {
			return NULL;
		}
		else {
			process_t *to_remove = temp->next;
			temp->next = temp->next->next;
			return to_remove;
		}
	}
}

/* adds element to schedule queue in ascending order of deadlie*/
void priority_enqueue(process_t *new_elem) {
	
	process_t *tmp = schedule_queue;
	
	if (schedule_queue == NULL) {
		schedule_queue = new_elem;
		new_elem -> next = NULL;
	}
	else if(get_time(tmp->deadline) >= get_time(new_elem->deadline)) {
		new_elem->next = tmp;
		schedule_queue = new_elem;
	}
	else {
		while (tmp->next != NULL && get_time(tmp->next->deadline) < get_time(new_elem->deadline)){
			tmp = tmp->next;
		}
		new_elem -> next = tmp -> next;
		tmp -> next = new_elem;
	}
}

/* adds element to the end of the prcoess queue */
void enqueue(process_t *new_elem) {
	if (process_queue == NULL) {
		process_queue = new_elem;
		new_elem -> next = NULL;
	}
	else {
		process_t *tmp = process_queue;
		while (tmp -> next != NULL) {
			tmp = tmp->next;
		}
		tmp -> next = new_elem;
		new_elem -> next = NULL;
	}
}

void master_enqueue(process_t *new_elem) {
	if(new_elem->deadline == NULL) {
		enqueue(new_elem);
	}
	else {
		priority_enqueue(new_elem);
	}
}

process_t* master_dequeue() {
	process_t* temp = priority_dequeue();
	if (temp != NULL) {
		return temp;
	}
	temp = process_queue;
	dequeue();
	return temp;
}

int process_create (void (*f)(void), int n) {
	
	//allocate space for a new process state and create it
	process_t *new_p = malloc(sizeof(process_t));
	if (new_p == NULL) { //if couldn't allocate enough space for stack
		return -1;
	}
		
	new_p -> sp = process_stack_init(*f, n);
	if (new_p -> sp == NULL) {
		return -1;
	}
	
	new_p -> next = NULL;
	new_p -> og_sp = new_p -> sp;
	new_p -> stack_size = n;
	new_p -> deadline = NULL;
	new_p -> start = NULL;
	new_p -> period = NULL;
	new_p -> relative_deadline = NULL;
	new_p -> pc = NULL;
	
	enqueue(new_p);
	
	return 0;	
}

int process_rt_create(void (*f)(void), int n, realtime_t *start, realtime_t *deadline) {
	//allocate space for a new process state and create it
	process_t *new_p = malloc(sizeof(process_t));
	if (new_p == NULL) { //if couldn't allocate enough space for stack
		return -1;
	}
		
	new_p -> sp = process_stack_init(*f, n);
	if (new_p -> sp == NULL) {
		return -1;
	}
	
	new_p -> next = NULL;
	new_p -> og_sp = new_p -> sp;
	new_p -> stack_size = n;
	new_p -> deadline = deadline;
	new_p -> start = start;
	new_p -> period = NULL;
	new_p -> relative_deadline = NULL;
	new_p -> pc = NULL;
	priority_enqueue(new_p);
	
	return 0;
}

int process_rt_periodic(void (*f)(void), int n, realtime_t *start, realtime_t *deadline, realtime_t *period) {
	//allocate space for a new process state and create it
	process_t *new_p = malloc(sizeof(process_t));
	if (new_p == NULL) { //if couldn't allocate enough space for stack
		return -1;
	}
		
	new_p -> sp = process_stack_init(*f, n);
	if (new_p -> sp == NULL) {
		return -1;
	}
	
	new_p -> next = NULL;
	new_p -> og_sp = new_p -> sp;
	new_p -> stack_size = n;
	realtime_t newT = {0, 1};
	
	//make new objects so pointers don't get messed up
	new_p -> deadline = malloc(sizeof(realtime_t));
	new_p -> start = malloc(sizeof(realtime_t));
	new_p -> period = malloc(sizeof(realtime_t));
	new_p -> relative_deadline = malloc(sizeof(realtime_t));
	
	//check we have enough space
	if (new_p -> deadline == NULL || new_p -> start == NULL || new_p -> period == NULL || new_p -> relative_deadline == NULL) {
		return -1;
	}
	
	//make assignments and enqueue
	new_p -> deadline -> sec = (get_time(start) + get_time(deadline))/1000;
	new_p -> deadline -> msec = (get_time(start) + get_time(deadline))%1000;
	new_p -> start -> sec = (get_time(start))/1000;
	new_p -> start -> msec = (get_time(start))%1000;
	new_p -> period -> sec = (get_time(period))/1000;;
	new_p -> period -> msec = (get_time(period))%1000;
	new_p -> relative_deadline -> sec = (get_time(deadline))/1000;;
	new_p -> relative_deadline -> msec = (get_time(deadline))%1000;
	new_p -> pc = f;
	priority_enqueue(new_p);
	
	
	return 0;
}

void process_start (void) {
	//timer 0 setup
	SIM->SCGC6 = SIM_SCGC6_PIT_MASK; // Enable clock to PIT module
	PIT->MCR = 0;
	PIT->CHANNEL[0].TFLG = 1;
	PIT->CHANNEL[0].LDVAL = 0x111111;
	PIT->CHANNEL[0].TCTRL |= 2;
	NVIC_EnableIRQ(PIT0_IRQn); /* enable PIT0 Interrupts */
	NVIC_SetPriority(PIT0_IRQn, 2);
	
	//timer 1 setup
	PIT->CHANNEL[1].TFLG = 1;
	PIT->CHANNEL[1].LDVAL = DEFAULT_SYSTEM_CLOCK/1000;
	PIT->CHANNEL[1].TCTRL |= 3;
	NVIC_EnableIRQ(PIT1_IRQn); /* enable PIT1 Interrupts */
	NVIC_SetPriority(PIT1_IRQn, 0);
	
	NVIC_SetPriority(SVCall_IRQn, 1);
	
	process_begin();
}

unsigned int * process_select(unsigned int * cursp) {
	
	//no current process
	if(current_process == NULL) {
		current_process = master_dequeue();
		if(current_process == NULL) {
			if (schedule_queue != NULL) {
				while (current_process == NULL) {
					current_process = master_dequeue();
				}
			}
			else {
				return NULL;
			}
		}
		return (*current_process).sp;
	}
	//current process has terminated
	else if(cursp == NULL) {
		//update met/missed deadline
		if (current_process -> deadline != NULL) {
			if (get_time(current_process->deadline) < current_time.sec*1000 + current_time.msec) {
				process_deadline_miss = process_deadline_miss + 1;
			}
			else {
				process_deadline_met = process_deadline_met + 1;
			}
		}
		
		//if periodic, reset the process
		if (current_process->period != NULL) {
			current_process -> start -> sec = (get_time(current_process->start) + get_time(current_process->period))/1000;
			current_process -> start -> msec = (get_time(current_process->start) + get_time(current_process->period))%1000;
			current_process -> deadline -> sec = (get_time(current_process->start) + get_time(current_process->relative_deadline))/1000;
			current_process -> deadline -> msec = (get_time(current_process->start) + get_time(current_process->relative_deadline))%1000;
			current_process->sp = current_process->og_sp;
			current_process-> sp= process_stack_reinit(*(current_process -> pc), current_process->stack_size, current_process->og_sp);
			current_process-> og_sp= current_process->sp;
			priority_enqueue(current_process);
		}
		else {
			process_stack_free(current_process -> og_sp, current_process -> stack_size);
			free(current_process);
		}
		
		current_process = master_dequeue();
		if(current_process == NULL) {
			if (schedule_queue != NULL) {
				while (current_process == NULL) {
					current_process = master_dequeue();
				}
			}
			else {
				return NULL;
			}
		}
		return (*current_process).sp;
	}
	//current process not done yet
	else {
		current_process -> sp = cursp;
		master_enqueue(current_process);
		current_process = master_dequeue();
		return (*current_process).sp;
	}
}

/* 
PIT1 Interrupt Handler
*/
void PIT1_IRQHandler(void) {
	
	NVIC_DisableIRQ(PIT0_IRQn);

	current_time.msec = current_time.msec + 1;
	if(current_time.msec == 999) {
		current_time.msec = 0;
		current_time.sec = current_time.sec + 1;
	}
	
	PIT->CHANNEL[1].TFLG = 1;  // reset timer
	
	NVIC_EnableIRQ(PIT0_IRQn);
}

