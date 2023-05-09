/*exceptions.h*/
#ifndef exceptions
#define exceptions

#include "types.h"
#include "/usr/include/umps3/umps/libumps.h"

extern void passUpOrDie(int exc);
extern void syscallHandler();
extern void tlbExcept();
extern void prgTrap();
extern void pOp(int* sem);
extern void vOp(int* sem);
extern void block(int* process);
extern void sys1();
extern void sys2();
extern void sys3();
extern void sy4();
extern void sys5();
extern void sys6(cpu_t time);
extern void sys7();
extern void sys8();

#endif
