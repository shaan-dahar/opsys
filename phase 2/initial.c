/*
* Initial.c
* This file serves 3 main purposes: identify exception types, initialize
* global variables that are used in running processes, and setting addresses for the
* TLB_Refill_Handler. It initializes PCBs and the ASL then calls the scheduler after 
* inserting a new process.
*
* Authors: Shaan Dahar, John Weisbrod
*/
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/umps3/umps/libumps.h"
#include "../h/p2test.h"
/*
* External Methods from exceptions and the test code
*/

extern void test();
extern void uTLB_RefillHandler();
extern void syscallHandler();
extern void prgTrap();

void genExceptHandler();

/*initialize globals*/
pcb_PTR ReadyQ;
pcb_PTR currentProc;
int procCount;
int SoftBlockCnt;
int Device_Sema[MAGICNUM];
state_t prevState; 

/*
FunctionName: main
Parameters: n/a
Return: n/a
Purpose: "Main" function call that will set all of the global variables to default values, 
	create processes, initialize the device semaphores pcbs and the ASL, then finally
	call the scheduler
*/ 
int main(){
	ReadyQ = mkEmptyProcQ(); /*make the readyQ empty*/
	currentProc = NULL; 
	procCount = 0;
	SoftBlockCnt = 0;
	int i;
	for (i = 0; i < MAGICNUM; i++) {
		Device_Sema[i] = 0;
	}	
	devregarea_t* dra = (memaddr) RAMBASEADDR;
	passupvector_t* puv = (passupvector_t*) PASSUPVECTOR;
	memaddr ramtop = (dra->rambase) + (dra->ramsize); 
	puv -> tlb_refll_handler = (memaddr) uTLB_RefillHandler;
	puv -> tlb_refll_stackPtr = KERNELSTACK;
	puv -> execption_handler = (memaddr) genExceptHandler;
	puv -> exception_stackPtr = KERNELSTACK;
	
	initPcbs();
	initASL();
	
	pcb_PTR p = allocPcb();
	p -> p_s.s_sp = (memaddr) ramtop;
 	p -> p_s.s_t9 = (memaddr) test; /*t9 does as pc does*/
	p -> p_s.s_pc = (memaddr) test;
	p -> p_s.s_status = OFF | ITRON | IPAREA | LCLTON; /*interrupts on, kernel mode, local timer on*/
	p -> p_supportStruct = NULL;
	insertProcQ(&(ReadyQ), p);
	procCount++;
	LDIT(IOCLOCK);
	schedule();
	
	
	return 0; 
}

/*
FunctionName: genExceptionHandler
Parameters: none
Return: none
Purpose: This will pass the state that raised the exception into the Bios DataPage,
	and take what ever cause code it raised to call the appropriate exception 
	function in exceptions.c
*/ 
void genExceptHandler(){
	state_PTR prevState = (state_PTR) BIOSDATAPAGE;
	int eCode = ((prevState -> s_cause & CAUSE) >> SHIFT);
	if (eCode == INT) {
		interruptHandler(); /*go to interrupts.c*/
	}
	if (eCode <= TLBEX) {
		tlbExcept(); /*passupordie*/
	}
	if (eCode == SYSC) {
		syscallHandler(); /*go to exceptions.c*/
	}
	else {
		prgTrap();  /*passupordie*/
	}
}
