/* ASL.C
* This file includes all of the functions that can manipulate the
* struct semd_t. It allows you to insert, remove (a head or any given element),
* check if empty, instantiate, or see the head of any of the following data
* structures: sorted array, unsorted array 
*
* Authors: Shaan Dahar, John Weisbrod
*/
#include "../h/types.h"
#include "../h/pcb.h"

semd_PTR semd_h;
semd_PTR semdFree_h;

/*
FunctionName: search
Parameters: semaphore we are looking for, semAdd
Return: parent of the semaphore
Purpose: find the semaphore in the active list
*/ 
semd_PTR search(int *semAdd){ /*return parent of semd with semAdd*/
	if (semAdd == NULL) {
		semAdd = (int*)(MAXINT);
	}
	semd_PTR temp = semd_h;
	while (semAdd > temp -> s_next -> s_semAdd) { /*while temp hasn't found child, and not at end*/
		temp = temp -> s_next; /*iterate*/
	}
	return temp;
}

/*
FunctionName: alloc
Parameters: none
Return: pointer to the previous semaphore in the active list
Purpose: adds element from semdFreeList, if empty return NULL
*/ 
semd_PTR alloc() {/*initializes values for semd for active list*/
	if (semdFree_h == NULL) { /*no free semd*/
		return NULL; /*return NULL*/
	} else { /*initialize values*/
		semd_PTR temp = semdFree_h;
		temp -> s_semAdd = NULL;
		semdFree_h = semdFree_h -> s_next;/*removes temp from free list*/
		temp -> s_procQ = mkEmptyProcQ(); /*empty proc Q on temp*/
		return temp;	
	}
}

/*
FunctionName: dealloc
Parameters: semaphore address that will be put in the freeList, semAdd
Return: none
Purpose: move semaphore from the active list to the free list
*/ 
void dealloc(semd_PTR semAdd) {/*moves sem from active to free list*/
	if (semdFree_h == NULL) {
		semdFree_h = semAdd; /*make first element in free list semAdd*/
		semdFree_h -> s_next = NULL; /*define end of free list*/
	}
	else {
		semAdd -> s_next = semdFree_h; /*make semAdd start of free list*/
		semdFree_h = semAdd; /*free list points to semAdd*/
	}
}

/*
FunctionName: initASL
Parameters: none
Return: none
Purpose: Initialize the semdFree list to contain all of the elements of the static semdTable
*/ 
void initASL (){
	static semd_t semdTable[MAXPROC+2]; /*semd_t array 22 long*/
	int i;
	semd_h = NULL;
	semdFree_h = NULL;
	for (i = 0; i < MAXPROC; i++){
		/*add stuff to the freelist*/
		dealloc(&(semdTable[i]));
	} /*free list initialized, now initialize active list*/
	semd_h = &(semdTable[MAXPROC+1]); /*set final member of table to start/end of active list*/
	semd_h -> s_next = NULL; /*define end of active list*/
	semd_h -> s_semAdd = (int*) MAXINT; /*end of active list, largest semAdd possible*/
	semd_h -> s_procQ = mkEmptyProcQ();
	
	(semdTable[MAXPROC]).s_next = semd_h; /*new semd_t poinst to start/end of active list*/
	semd_h = (&(semdTable[MAXPROC])); /*new semd_t is first element of active list*/
	semd_h -> s_semAdd = 0; /*lowest number possible for semAdd*/
	semd_h -> s_procQ = mkEmptyProcQ();
	semd_h -> s_next -> s_next = NULL; /*final member of active list points to NULL*/
	return;
}

/*
FunctionName: insertBlocked
Parameters: pcb to be inserted, p; semaphore that receives new pcb, semAdd
Return: TRUE is new sempahore needs to be added, else FALSE
Purpose: add a new pcb into a given sempahore and let the system know if a new semaphore needs to be created
*/ 
int insertBlocked (int *semAdd, pcb_PTR p){
	semd_PTR temppar = search(semAdd); /*search semd_h*/
	if (temppar -> s_next -> s_semAdd != semAdd) { /*sem wasn't found*/
		semd_PTR new = alloc(); /*allocate new semd*/			
		if (new == NULL) { /*if no space in freelist*/
			return TRUE; /*is blocked*/
		}
		else { /*space in free list*/
			new -> s_next = temppar -> s_next; /*new's next is parent's next*/
			temppar -> s_next = new; /*new becomes child of parent*/
			p -> p_semAdd = semAdd; /*sem & p need address set*/
			new -> s_semAdd = semAdd; 
			insertProcQ(&(new -> s_procQ), p); /*not blocked, call insertProcQ*/
			return FALSE;
		}			
	} else {/*if sem found*/
		p -> p_semAdd = semAdd; 
		insertProcQ(&(temppar -> s_next -> s_procQ), p);/*insertprocQ*/
		return FALSE;
	}
}

/*
FunctionName: removeBlocked
Parameters: semaphore whose head element will be removed, semAdd
Return: pcb_PTR that is removed via removeProcQ; NULL if no semaphore is found
Purpose: This will look for the given semaphore and remove the head element associated with the semaphore. If no such 
	 sempahore exists this will return NULL
*/ 
pcb_PTR removeBlocked (int *semAdd){
	semd_PTR removeNext = search(semAdd);
	if (removeNext ->s_next -> s_semAdd != semAdd) { /*case 1: if the semd doesn't exist */
		return NULL;						       /*or if the procQ is empty*/
	}
	semd_PTR semRem = removeNext -> s_next;	/*case 2: if it exists call removeProcQ to remove the head element*/
	pcb_PTR removeThis = (removeProcQ(&(semRem -> s_procQ))); 
	if (emptyProcQ(removeNext -> s_next -> s_procQ)) { /*send the semd to the free list if its Q is now empty*/
		removeNext -> s_next = semRem -> s_next; /*ASL should skip the removed semd_t*/
		dealloc(semRem); /*deallocate semd_t*/
	}
	return removeThis; /*return the procQ*/
}

/*
FunctionName: outBlocked
Parameters: pcb_PTR p to be removed via outProcQ
Return: removed pcb_PTR
Purpose: This will remove the pcb pointed to by p's semaphore on the ASL. If no such pcb exits, return NULL
*/ 
pcb_PTR outBlocked (pcb_PTR p){
	int *semAdd = p -> p_semAdd;
	semd_PTR removeNext = search(semAdd);
	semd_PTR semRem = removeNext -> s_next;
	if (removeNext ->s_next -> s_semAdd != semAdd) { /*case 1: semd doesn't exist*/
		return NULL;
	}
	pcb_PTR removeThis = outProcQ(&(semRem -> s_procQ), p); /*if it exists then remove the pcb child via outProcQ*/
	if (emptyProcQ(removeNext -> s_next -> s_procQ)) { /*if the semd's Q is now empty then deallocate it*/
		removeNext -> s_next = semRem -> s_next; /*ASL should skip the removed semd_t*/
		dealloc(semRem);
	}
	return removeThis;
}

/*
FunctionName: headBlocked
Parameters: semaphore to remove head element of associated queue, semAdd
Return: pcb_PTR of the given semaphore
Purpose: This will find the headd element of a given semaphore, and return NULL if the semAdd doesn't
	 exist or if the queue in it is empty
*/ 
pcb_PTR headBlocked (int *semAdd){
	semd_PTR semNext = search(semAdd);
	if (semNext -> s_next -> s_semAdd != semAdd) {/*case 1: parent or semd doesn't exist*/
		return NULL;
	}
	return headProcQ(semNext -> s_next -> s_procQ);
}
