/*
**********INTERRUPTS.C***********
* Authors: Shaan Dahar and John Weisbrod
* This file contains two functions that will identify the device and line numbers 
* that cause any interrupt. When a quantum ends, it will put a proccess on the readyQ and 
* call the scheduler. When an interrupt happens when the line number needs to be identified
* it will pass the state into v0, identify the status and move on. 
*/
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/const.h"
#include "../h/types.h"
#include "../h/asl.h"
#include "../h/pcb.h"
#include "/usr/include/umps3/umps/libumps.h"
/* globals */
extern pcb_PTR ReadyQ;
extern pcb_PTR currentProc;
extern int procCount;
extern int SoftBlockCnt;
extern int Device_Sema[MAGICNUM];
extern cpu_t TODStart;

/*
FunctionName: tDevHandler
Parameters: interrupted device semaphore, dSem
Return: state of interrupt, state
Purpose: This is meant to acknowledge that the interrupt was generated. IF the terminal was ready change the status
*/ 
int tDevHandler(int *dSem) {
	volatile devregarea_t* dR = (devregarea_t *)RAMBASEADDR;
        unsigned int state;
 	if ((dR -> devreg[(*dSem)].t_transm_status & 0x0F) != READY) { /*if terminal device is not ready*/
 		state = dR-> devreg[(*dSem)].t_transm_status;
 		dR -> devreg[(*dSem)].t_transm_command = ACK; /*acknowledge it*/
 	}
 	else { /*if ready*/
 		state = dR-> devreg[(*dSem)].t_recv_status;
		dR -> devreg[(*dSem)].t_recv_command = ACK;
		*dSem += DEVPERINT;
 	}
 	return state;
}

/*
FunctionName: devHandler
Parameters: interrupted device semaphore, dSem
Return: state of interrupt, state
Purpose: This means to acknowledge the interrupt that was generated
*/ 
int devHandler(int *dSem) {
	volatile devregarea_t* dR = (devregarea_t *)RAMBASEADDR;
	unsigned int state = dR-> devreg[(*dSem)].d_status;
	dR -> devreg[(*dSem)].d_command = ACK;
	return state;
}

/*
FunctionName: intHelp
Parameters: device semphore number, t
Return: none
Purpose: This is a helper function for the interruptHandler function. If the function found that either
	the disk, flash, or terminal numbers did not = 0 (turned on in order of priority), then the deviceInt was passed 
	here where we can determine its line number, state, and pass it into V0
	
*/ 
void intHelp(int t) {
        volatile devregarea_t* devReg = (devregarea_t*) RAMBASEADDR;
        unsigned int bitMap = (devReg->interrupt_dev[t-LINETODEV]); /*get the correct line's bitmap*/
        int dSem;
        int dNum;
        /*Find which device number, if not 1-6 default to 7*/
        if((bitMap & DEV0) != 0){
   	     dNum = 0;
    	}else if((bitMap & DEV1) != 0){
   	     dNum = 1;
    	}else if((bitMap & DEV2) != 0){
   	     dNum = 2;
    	}else if((bitMap & DEV3) != 0){
   	     dNum = 3;
    	}else if((bitMap & DEV4) != 0){
   	     dNum = 4;
   	 }else if((bitMap & DEV5) != 0){
   	     dNum = 5;
   	 }else if((bitMap & DEV6) != 0){
   	     dNum = 6;
   	 }else if ((bitMap & DEV7) != 0){
   	     dNum = 7;
   	 }
   	 else {
   	 	dNum = 7;
   	 }
   	 dSem = (((t - DISKINT)*DEVPERINT)+dNum);
   	 unsigned int state;
   	 if (t == TERMINT) { /*if terminal*/
   	 	state = tDevHandler(&dSem);   
   	 }	 
   	 /*just acknowledge this stupid interrupt*/   	 
   	 else {
   	 	state = devHandler(&dSem);
   	 }
   	 Device_Sema[dSem]++;
   	 if (Device_Sema[dSem] <= 0) { /* vOp on the interrupting device*/
        	pcb_PTR pointer = removeBlocked(&(Device_Sema[dSem]));
        	if (pointer != NULL) {
        		pointer->p_s.s_v0 = state;
        		insertProcQ(&ReadyQ, pointer);
        		SoftBlockCnt--;
        	}
   	 }
}


/*
FunctionName: interruptHandler
Parameters: none
Return: none
Purpose: This handles all interrupts and determine what device 
	generated the interrupt, and will either put the process back on the readyQ 
	and call the scheduler, or call intHelp to see what line number in the device
*/

void interruptHandler() {
	cpu_t timeStop;
	cpu_t timeRest; 
	STCK(timeStop);
	timeRest = getTIMER(); /*get remaining time for process*/
	int c = (((state_PTR) BIOSDATAPAGE) -> s_cause);
	if ((c & PLT)  != 0) { /*process local timer interrupt*/
		/*PLT interupt, quantum is up, switch to next process*/
		/*get proc time, store state, put proc on ReadyQ*/
		if (currentProc != NULL){
			moveState(((state_PTR) BIOSDATAPAGE), &(currentProc->p_s)); /*save off old state*/
			insertProcQ(&ReadyQ, currentProc); /*go back to queue*/
			setTIMER(QUANTA); /*5 msec on clock*/
			schedule(); /*get next proc now*/
		}else { /*this should never ever happen what on god's earth did you do*/
			PANIC();
		}
	}
	if ((c & TIME) != 0) { /*pseudo clock tick, wake up everyone here and reset timer*/
		LDIT(ITVTIME); /*reset timer to 100ms */
		pcb_PTR p = removeBlocked(&Device_Sema[MAGICNUM - 1]); /*get the head pcb at pseudo clock semaphore*/
		while (p != NULL) {
			insertProcQ(&ReadyQ, p); /*put unblocked on readyQ*/
			SoftBlockCnt--; /*decrement for every pcb in interval timer sem that's unblocked*/
			p = removeBlocked(&Device_Sema[MAGICNUM - 1]); /*p is next blocked pcb from sem*/
		}
		Device_Sema[MAGICNUM - 1] = 0; /*reset interval timer to 0*/
		if (currentProc == NULL) {
			schedule(); /*if no current process then wait for next interrupt*/
		}
		else {
			currentProc -> p_time += (timeStop - TODStart); /*update time*/
			switchContext(currentProc); /*go back to current process*/
		}
	}
	/*If lines 3-7*/
	if ((c & DISK) != 0) {
		intHelp(DISKINT);
	}
	if ((c & FLASH) != 0) {
		intHelp(FLASHINT);
	}
	if ((c & NET) != 0) {
		intHelp(NETWINT);
	}
	if ((c & PRNT) != 0 ) {
		intHelp(PRNTINT);
	}
	if ((c & TERM) != 0) {
		intHelp(TERMINT);
	}
	/*return from intHelp to here*/
	if (currentProc != NULL) {
		currentProc -> p_time += (timeStop - TODStart); 
		moveState(((state_PTR) BIOSDATAPAGE), &(currentProc->p_s));
		switchContext(currentProc); /*go back to current process with remaining time on clock*/
	}
	if (currentProc == NULL) {
		schedule();
	}
	
}
