#ifndef scheduler
#define scheduler

/************************* SCHEDULER.H *****************************
*
*  The externals declaration file for the Scheduler File.
*
*/

extern void ReadyTimer(pcb_PTR contProc, cpu_t time);
extern void schedule(int a);
extern void moveState(state_PTR source, state_PTR dest);
extern void switchContext(pcb_PTR currProc);

extern cpu_t TODStart;

#endif
