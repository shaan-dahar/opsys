/*
* scheduler.c
* This file will control the execution of incoming processes. When deciding what to do next (what processes it has),
* this will decide whether or not to WAIT, HALT, or PANIC. Furthermore, it will moveStates
* for later reference, run the next process, and set the timer for running processes.
* Authors: Shaan Dahar, John Weisbrod
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

extern pcb_PTR ReadyQ;
extern pcb_PTR currentProc;
extern int SoftBlockCnt;
extern int procCount;

cpu_t TODStart;
cpu_t passed;

/*
FunctionName: ReadyTimer
Parameters: current process, contproc; quantum, time
Return: n/a
Purpose: prepare to switchcontext and setting the time to run (quantum)
*/ 
void ReadyTimer(pcb_PTR contProc, cpu_t time) {
	setTIMER(time);
	switchContext(contProc); 
}

/*
FunctionName: scheduler
Parameters: none
Return: none
Purpose: This will run the next process (remove it from the ready queue),
	then HALT, PANIC, or WAIT depending on other processes
*/ 
void schedule(){
	if (currentProc != NULL) { /*when current proc exists, add time elapsed to the current proc*/
		cpu_t passed;
		STCK(passed);
		currentProc->p_time += (passed - TODStart);
	}
	pcb_PTR nextProc = removeProcQ(&(ReadyQ)); /*get next thing to run*/
	if (nextProc != NULL) {
		ReadyTimer(nextProc, QUANTA); /*load 5 ms on clock, then switch context to new process*/
	}
	if (procCount >= 1) {
		if (SoftBlockCnt <= 0) {
		/*deadlock, shit hit the fan*/
			PANIC();
		} else { /*softblockcount > 0*/
		/*currently no ready jobs, wait for one to be ready*/ 
			currentProc = NULL;
			setSTATUS(OFF | IPAREA | ITCON); /*local timer removed, also works to set huge time on timer*/
			WAIT();
		}	
	}
	else if (procCount == 0) {
	/*get a beer, no incoming processes*/
		HALT();
	}
	else { /*negative number of processes*/
		PANIC();
	} 
	
}

/*
FunctionName: moveState
Parameters: state source, source, state destination, dest
Return: none
Purpose: Move a given state's pc, cause, status, and entryHi registers into another state storage
*/ 
void moveState(state_PTR source, state_PTR dest) {
	/*deep copy source to destination*/
	int i;
	for (i =0; i < STATEREGNUM; i++) {
		(dest->s_reg[i]) = (source -> s_reg[i]); 
	}
	(dest->s_entryHI) = (source->s_entryHI);
	(dest->s_cause) = (source->s_cause);
	(dest->s_pc) = (source->s_pc);
	(dest->s_status) = (source->s_status);
}

/*
FunctionName: switchContext
Parameters: current process, currProc
Return: none
Purpose: switch current process to whatever needs to run next, load the processor state and move along
*/ 
void switchContext(pcb_PTR currProc) {
	STCK(TODStart);
	currentProc = currProc;
	LDST(&(currProc->p_s));
}
