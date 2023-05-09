#ifndef initial
#define initial

#include "const.h"
#include "types.h"
#include "pcb.h"
#include "asl.h"
#include "interrupts.h"
#include "scheduler.h"
#include "exceptions.h"
#include "p2test.h"
/************************* INITIAL.H *****************************
*
*  The externals declaration file for the Initial Declaration File.
*
*/
extern int main();
extern void genExceptionHandler();

#endif
