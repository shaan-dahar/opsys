#ifndef CONSTS
#define CONSTS

/**************************************************************************** 
 *
 * This header file contains utility constants & macro definitions.
 * 
 ****************************************************************************/

/* Hardware & software constants */
#define PAGESIZE		  4096			/* page size in bytes	*/
#define WORDLEN			  4				  /* word size in bytes	*/
#define DEVICES 49

/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR		0x10000000
#define RAMBASESIZE		0x10000004
#define TODLOADDR		  0x1000001C
#define INTERVALTMR		0x10000020	
#define TIMESCALEADDR	0x10000024
#define DEVREGSTART 0x10000040

/* utility constants */
#define	TRUE			    1
#define	FALSE			    0
#define HIDDEN			  static
#define EOS				    '\0'
#define MAXPROC			20
#define NULL 			    ((void *)0xFFFFFFFF)
#define MAXINT				0xFFFFFFFF
#define LINETODEV 			3

#define INT				0 /*interrupt*/
#define TLBEX				3 /*tlb exception*/
#define SYSC 				8 /*syscall*/
#define MAGICNUM 			49
/* device interrupts */
#define PLTINT			1 /*process local timer interrupt*/
#define PLT 	0x00000200
#define TIMEINT			2 /*interval timer bus interrupt*/
#define TIME 	0x00000400
#define DISKINT			  3
#define DISK	0x00000800
#define FLASHINT 		  4
#define FLASH	0x00001000
#define NETWINT 		  5
#define NET 	0x00002000
#define PRNTINT 		  6
#define PRNT	0x00004000
#define TERMINT				7
#define TERM	0x00008000
#define ITVTIME 		100000 /*interval timer 100 ms*/
#define DEVINTNUM		  5		  /* interrupt lines used by devices */
#define DEVPERINT		  8		  /* devices per interrupt line */
#define DEVREGLEN		  4		  /* device register field length in bytes, and regs per dev */	
#define DEVREGSIZE	  16 		/* device register size in bytes */

/*constants to find interrupting devices*/
#define DEV0	0x00000001
#define DEV1	0x00000002
#define DEV2	0x00000004
#define DEV3	0x00000008
#define DEV4	0x00000010
#define DEV5	0x00000020
#define DEV6	0x00000040
#define DEV7	0x00000080

/* device register field number for non-terminal devices */
#define STATUS			  0
#define COMMAND			  1
#define DATA0			    2
#define DATA1			    3

/* device register field number for terminal devices */
#define RECVSTATUS  	0
#define RECVCOMMAND 	1
#define TRANSTATUS  	2
#define TRANCOMMAND 	3

/* device common STATUS codes */
#define UNINSTALLED		0
#define READY			    1
#define BUSY			    3

/* device common COMMAND codes */
#define RESET			    0
#define ACK				    1

/* Memory related constants */
#define KSEG0           0x00000000
#define KSEG1           0x20000000
#define KSEG2           0x40000000
#define KUSEG           0x80000000
#define RAMSTART        0x20000000
#define BIOSDATAPAGE    0x0FFFF000
#define PASSUPVECTOR	  0x0FFFF900
#define IOCLOCK 100000
#define QUANTA 5000
#define MAXTIME 0xFFFFFFFF
#define RAMTOP 0x20001000

/*bit manipulation constants*/
#define OFF 	 0x00000000 /*blank slate for setting status and what not*/
#define SHIFT	 0x00000002 /*shifting constant*/
#define IPSHIFT 0x00000008  /*shifting for interrupts*/
#define CAUSE	 0x0000007C /*get cause code before shifting*/
#define ITRON	 0x00000004 /*interrupts previous bit on*/
#define ITCON	 0x00000001 /*interrupt current bit on, & w/ off to turn off*/
#define KCON	 0x00000002 /*kernel current bit, & with OFF to go usermode*/
#define KON 	 0x00000008 /*kernel previous bit*/
#define IPAREA	 0x0000FF00 /*interrupt pending area, & with cause register*/
#define LCLTON	 0x08000000 /*timer on*/
#define NEXT    0x00000004 /*pc next instruction*/

/* Exceptions related constants */
#define	PGFAULTEXCEPT	  0
#define GENERALEXCEPT	  1


/* operations */
#define	MIN(A,B)		((A) < (B) ? A : B)
#define MAX(A,B)		((A) < (B) ? B : A)
#define	ALIGNED(A)		(((unsigned)A & 0x3) == 0)

/* Macro to load the Interval Timer */
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR))) 

/* Macro to read the TOD clock */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))

#endif
