/*
exceptions.c
This file will handle all excpetions. This mainly handles sysCalls 1-8 as each gets its own function.
If it is not a syscall in 1-8 then it is found to be either a program trap or sys9+ in which case it is treated as passUpOrDie,
in which case the process and all of its children in the process tree will be terminated, or pass up the support struct to the BIOS data page.
Authors: Shaan Dahar, John Weisbrod
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
/*globals*/
extern pcb_PTR ReadyQ;
extern pcb_PTR currentProc;
extern int procCount;
extern int SoftBlockCnt;
extern int Device_Sema[MAGICNUM];
extern cpu_t TODStart;
extern cpu_t passed;
/*
FunctionName: pOp (wait)
Parameters: semaphore, sem
Return: none
Purpose: This is the wait function, that will tell a seamphore "if no one else is going then I will"
*/ 
void pOp(int* sem) {
	(*sem)--;
	if ((*sem) < 0) {
		cpu_t TODStop;
		STCK(TODStop);
		currentProc -> p_time += (TODStop - TODStart);
		insertBlocked(sem, currentProc);
		currentProc = NULL;
		schedule();
	}
	else {
		switchContext(currentProc);
	}
}
/*
FunctionName:  vOp (signal)
Parameters: semaphore, sem
Return: none
Purpose: This is the signal function where a semphore is "done, someone else go"
*/ 
void vOp(int* sem) {
	(*sem)++;
	if ((*sem) <= 0){
		pcb_PTR p = removeBlocked(sem);
		if (p != NULL) {
			insertProcQ(&ReadyQ, p);
		}
	}
	switchContext(currentProc);
}

/*
FunctionName: block
Parameters: process to block, proc
Return: none
Purpose: block a process to wait for I/O interrupts
*/ 
void block(int* proc) {
	SoftBlockCnt++; /*increment softblock*/
	cpu_t TODStop;
	STCK(TODStop);
	currentProc -> p_time += (TODStop - TODStart);  /*add time to current process*/
	insertBlocked(proc, currentProc); /*block it*/
	currentProc = NULL;
	schedule(); /* get new process*/
}

/*
FunctionName: sys1
Parameters: none
Return: none
Purpose: Create_Process, when the service calls a new process this will be called
Registers: a0, 1; a1, pointer to processor state, v0, -1 if no free PCBs else 0
*/ 
void sys1 () {
	pcb_PTR n = allocPcb();
	if (n != NULL) {
		procCount++;
		moveState( (state_PTR) currentProc->p_s.s_a1, &(n->p_s));
		currentProc -> p_s.s_v0 = 0;
		n -> p_supportStruct = ((support_t *) currentProc -> p_s.s_a2);
		insertProcQ(&ReadyQ, n);
		insertChild(currentProc, n);
	}
	else{
		currentProc -> p_s.s_v0 = -1;
	}
	/*always return to current process*/
	switchContext(currentProc); 
}

/*
FunctionName: sys2
Parameters: none
Return: none
Purpose: Terminate, when we want to assassinate the current process and it's bloodline
Registers: a0, 2
*/ 
void sys2 () {
	/*nuke tree and find new process*/
	term(currentProc);
	schedule();
}

/*
FunctionName: sys3
Parameters: none
Return: none
Purpose: Wait, want to perform P on a semaphore
Registers: a0, 3; a1 physical address of semaphore
*/ 
void sys3(){
	pOp((int *)currentProc->p_s.s_a1);
}

/*
FunctionName: sys4
Parameters: none
Return: none
Purpose: Signal, want to perform V on a semaphore
Registers: a0, 4; a1, physical address of semaphore
*/ 
void sys4(){
	vOp((int *)currentProc->p_s.s_a1);
}

/*
FunctionName: sys
Parameters: none
Return: none
Purpose: Want to wait for I/O, block device and its line number
Registers: a0, 5; a1, line number; a2, device number
*/ 
void sys5(){
	int line = currentProc->p_s.s_a1;
        int dev = currentProc -> p_s.s_a2;
        dev += ((line - LINETODEV) * DEVPERINT); /*this is sema4 of device w/ interrupt*/
        if (((currentProc->p_s.s_a3)) && (line == TERMINT)) {
                dev += DEVPERINT; /*add 8 to get the right device for a terminal*/
	}
	Device_Sema[dev]--;
	block(&(Device_Sema[dev]));
}

/*
FunctionName: sys6
Parameters: 
Return: none
Purpose: get the accumulated processor time and put it in V0
Registers: a0, 6; v0, CPU_TIME
*/ 
void sys6(){
	cpu_t time;
	STCK(time);
	currentProc->p_time += (time - TODStart);
	currentProc-> p_s.s_v0 = currentProc -> p_time;
        currentProc->p_s.s_pc += NEXT;
	switchContext(currentProc); 
}

/*
FunctionName: sys7
Parameters: none
Return: none
Purpose: Wait for clock, P the pseudo-clock that will be V ed in 100ms
Registers: a0, 7
*/ 
void sys7(){
	Device_Sema[MAGICNUM-1]--;
	block(&(Device_Sema[MAGICNUM-1]));
	schedule();
}

/*
FunctionName: sys8
Parameters: none
Return: none
Purpose: getSupportData, get pointer to current process support struct
Registers: a0, 8
*/ 
void sys8(){
	currentProc->p_s.s_v0 = (support_t*) currentProc->p_supportStruct;
	switchContext(currentProc);
}
/*
FunctionName: term
Parameters: pointer to tree to be nuked, p
Return: none
Purpose: when we want to execute a process, (sys2 or die), we will call this function
	which will kill the process and all of its children.
*/
void term(pcb_PTR p) {
	pcb_PTR op;
	/* while the process still has a bloodline*/
	while (!emptyChild(p)) {
		term(removeChild(p));
	}
	/*kill the current process*/
	if (p == currentProc) {
		outChild(p);
	}else if (p -> p_semAdd == NULL) { /*no associated semaphore*/
		outProcQ(&ReadyQ, p);
	}else {
		op = outBlocked(p);
		if (op != NULL) {
			int* tSem = op -> p_semAdd;
			if (tSem >= &Device_Sema[0] && tSem <= &Device_Sema[DEVICES-1]) { /*if not the clock or final device*/
				SoftBlockCnt--;
			}
			else {
				(*tSem)+= 1;
			}
		}
	}
	procCount--;
	freePcb(p);
}
/*
FunctionName: passUpOrDie
Parameters: exceptionType, exc
Return: none
Purpose: For Sys9+, this will either pass up the support struct (pass up) or nuke
the tree as if it were a sys2 and call term (die)
*/ 
void passUpOrDie(int exc) {
		/*pass up*/
                if(currentProc -> p_supportStruct != NULL) {
                        /*copy over state into current proc state, then update pc*/
                        moveState((state_t*) BIOSDATAPAGE, &(currentProc->p_supportStruct->sup_exceptState[exc]));
                        LDCXT(currentProc->p_supportStruct->sup_exceptContext[exc].c_stackPtr,
                        currentProc->p_supportStruct->sup_exceptContext[exc].c_status,
                        currentProc->p_supportStruct->sup_exceptContext[exc].c_pc);
                }else{
                                 /*terminate the process, aka death*/
		   term(currentProc);
		   schedule();
                }
               
}

/*
FunctionName: syscallHandler
Parameters: none
Return: none
Purpose: This function will take the state of the BIOSDATAPAGE and load a0 with the sysNumber
	update the PC, and if this is not a program trap (kernelMode = 1 or sysNum > 8),
	then it will call the appropriate sysCall function based on its number (1-8)
*/ 
void syscallHandler() {
	state_PTR state = (state_PTR) BIOSDATAPAGE; 
        int kernelMode = (currentProc->p_s.s_status & KON);
        int sysNum = state->s_a0;
        if (kernelMode == 1) {
        	passUpOrDie(GENERALEXCEPT);
        }
        moveState(state, &(currentProc -> p_s));
	/*update PC*/
	if (sysNum != 2) {
		currentProc->p_s.s_pc += NEXT;
	}
	
	/*for all 8 sysCalls, determine the number and call that sysCall's helper function,
	else passUpOrDie*/
        switch(sysNum){
	case 1:
		sys1();
		break;
	case 2:
		sys2();
		break;
	case 3:
		sys3();
		break;
	case 4:
		sys4();
		break;
	case 5:
		sys5();
		break;
	case 6:
		sys6();
		break;
	case 7:
		sys7();
		break;
	case 8:
		sys8();
		break;
	default:
		passUpOrDie(GENERALEXCEPT);
	}
}

/*
FunctionName: tlbExcept
Parameters: n/a
Return: n/a
Purpose: called by general exception handler, just passuporDie a pagefault
*/ 
void tlbExcept() {
	passUpOrDie(PGFAULTEXCEPT);
}

/*
FunctionName: prgTrap
Parameters: n/a
Return: n/a
Purpose: called by general exception handler, just passupordie for a program trap
*/ 
void prgTrap() {
	passUpOrDie(GENERALEXCEPT);
}
