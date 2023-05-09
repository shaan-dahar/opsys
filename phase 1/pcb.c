/* PCB.C
* This file includes all of the functions that can manipulate the
* struct pcb_t. It allows you to insert, remove (a head or any given element),
* check if empty, instantiate, or see the head of any of the following data
* structures: queue, process tree. 
*
* Authors: Shaan Dahar, John Weisbrod
*/
#include "../h/pcb.h"
 /*
HIDDEN pcb_t p;
HIDDEN pcb_PTR tp;
 */
pcb_PTR pcb_freeH;


void debug(int a) {
	return;
}

/*
FunctionName: allocPcb
Parameters: none
Return: Pointer of removed element
Purpose: remove element from pcbFreeList, if empty return NULL
*/ 
pcb_PTR allocPcb() { /*return null if free list empty*/
                if (pcb_freeH == NULL) {
                                return NULL;
                }
                else { /*initialize values and return a free PCB from list*/
                		 pcb_PTR temp = pcb_freeH; /*get pointer to first thing in free list*/
                		 pcb_freeH = pcb_freeH -> p_next; /*iterate in free list*/
                                (temp -> p_next) = NULL;
                                (temp -> p_prev) = NULL;
                                (temp -> p_prnt) = NULL;
                                (temp -> p_child) = NULL;
                                (temp -> p_sibPrev) = NULL;
                                (temp -> p_sibNext) = NULL;
                                (temp -> p_semAdd) = NULL;
                                (temp -> p_time) = 0;
                                temp -> p_supportStruct = NULL;
                                return temp;
                }     
}

/*
FunctionName: mkEmptyProcQ
Parameters: none 
Return: pointer (should always be null)
Purpose: Return tail pointer of empty queue which should be null
*/ 
pcb_PTR mkEmptyProcQ(){ /*return null*/
                return NULL;
}
 
/*
FunctionName: emptyProcQ
Parameters: tail pointer, tp
Return: boolean
Purpose: Return True if the queue pointed to by tp is empty
*/ 
int emptyProcQ(pcb_PTR tp){ /*checks if tp has empty proc Q*/
                return (tp == NULL);
}

/*
FunctionName: insertProcQ
Parameters: tail pointer, tp; inserted pcb pointer, p
Return: none
Purpose: Insert pcb pointed to by 'p' into the queue. If the queue is empty, make p the first element, else just add it.
*/ 
void insertProcQ(pcb_PTR *tp, pcb_PTR p){
                /*case 1, empty queue*/
                if (emptyProcQ(*tp)) {
                                p->p_next = p; /*make it point to itself*/
                                p->p_prev = p;
                                *(tp) = p; /*head is tail*/
                                return;
                }
                else { /*non-empty queue*/
                if ((*tp) == p) { /*head is tail*/
                	(*tp) -> p_prev = (*tp) -> p_next = p; /*both point to each other*/
                	p -> p_next = p -> p_prev = (*tp);
                	(*tp) = p;                	
                }
                else {
                (*tp) -> p_next -> p_prev = p; /*head's previous should be p*/
                p -> p_prev = (*tp); /*p's previous should be tp*/
                p -> p_next = (*tp) -> p_next; /*p's next is head*/
                (*tp) -> p_next = p; /*tp's next should be p*/
                (*tp) = p; /*p is the new tail*/
                }
                return;
                }             
}


/*
FunctionName: headProcQ
Parameters: tail pointer, tp
Return: pointer to head 
Purpose: If queue is empty return NULL, else return the pointer to the head element in the queue
*/  
pcb_PTR headProcQ (pcb_PTR tp) {
	if (emptyProcQ(tp)) {
		return NULL; /*if empty queue, return NULL*/
	}
	pcb_PTR temp = tp -> p_next;
	return temp; /*return tp's next (head pointer)*/
}

/*
FunctionName: removeProcQ
Parameters: tail pointer, tp
Return: pointer to popped element, pcb_PTR
Purpose: Remove the head element (FIFO) from the queue. If the queue is empty return NULL,
         else remove the head and return it.
*/ 
pcb_PTR removeProcQ (pcb_PTR *tp) {
                if (emptyProcQ(*tp)) { /*check if procQ is empty*/
                                return NULL;
                }
                else {
                	pcb_PTR head = ((*tp) -> p_next);
                	if ((*tp) -> p_next == (*tp)) { /*is head the only element in the list*/
                			 (*tp) = mkEmptyProcQ();
                       	         return head;
                	}
                	else { /* more than one element in list*/
                	head -> p_next -> p_prev = (*tp);
                	((*tp) -> p_next) = head -> p_next; 
                	}
                	return head;
                }
}

/*
FunctionName: outProcQ
Parameters: tail pointer pointer, tp; desired element, p
Return: pointer to removed element, p
Purpose: Instead of just removing the head, this function removes an element from anywhere in the queue.
         Return NULL if queue is empty. 
*/ 
pcb_PTR outProcQ (pcb_PTR *tp, pcb_PTR p) {
		pcb_PTR temp;
                if (emptyProcQ(*tp) || p == NULL) { /*case 1: empty queue*/
                                return NULL;
                }
                if (headProcQ((*tp)) == p) {/*case 2: p is the head*/
                                return removeProcQ(tp); /*just save time and remove tail's next*/
                }
                else { /* case 3: somewhere in the queue*/
                	if ((*tp) == p) {
                		((*tp) -> p_prev -> p_next) = (*tp) -> p_next;
                       	((*tp) -> p_next -> p_prev) = (*tp) -> p_prev;
                		return *tp;
                	}
                	temp = (*tp) -> p_prev; /*start after tail*/
                        while (temp != (*tp)) { /* search through the queue until you find it */ 
                        	if (temp == p){
                                        (temp -> p_prev -> p_next) = temp -> p_next;
                                        (temp -> p_next -> p_prev) = temp -> p_prev;
                                        return temp;
                        	}
                        	temp = (temp -> p_prev);
                        }
                        return NULL; /*case 4: it wasn't in there, return NULL*/
                }
}
/*
FunctionName: emptyChild
Parameters: pointer to pcb, p
Return: boolean
Purpose: Return True if p has no children, else false
*/               
int emptyChild (pcb_PTR p) {
                return (p->p_child == NULL);
}

/*
FunctionName: insertChild
Parameters: pointer to pcb parent node, prnt; pointer to pcb to be inserted, p
Return: none
Purpose: Have the parent node pointed to by prnt, point to a new child pointed to by p
*/                
void insertChild (pcb_PTR prnt, pcb_PTR p) {
                if (!emptyChild(prnt)) { /*if child exists*/
                                prnt -> p_child -> p_sibPrev = p; /*make p a previous sibling*/
                                p -> p_sibNext = prnt -> p_child; /*p is first sibling*/
                                p -> p_prnt = prnt;
                                prnt -> p_child = p;
                		 return;
                }
                else { /*no child*/
                	p -> p_prnt = prnt; 
                	prnt -> p_child = p;
                	p -> p_sibNext = NULL;
                	p -> p_sibPrev = NULL;
                	return;
                }
}             

/*
FunctionName: removeChild
Parameters: pointer to pcb that will be removed, p
Return: removed pointer, p
Purpose: Remove the head child of pcb pointed by p and return it. If p had no children, return NULL
*/ 
pcb_PTR removeChild (pcb_PTR p) {
                /* case 1, no child*/
                if (emptyChild(p)) {
                                return NULL;
                }
                else {
		 	pcb_PTR temp = (p -> p_child);
                	/*case 2, child*/
                	if ((temp -> p_sibNext) != NULL) { /*siblings exist*/
                       	         p -> p_child = p -> p_child -> p_sibNext;
                       	         p -> p_child -> p_sibPrev = NULL;
                	}
                	else { /* sibings don't exist*/
                			 p -> p_child = NULL;
                	}
                	temp -> p_prnt = NULL;
                       temp -> p_sibNext = NULL;
                       temp -> p_sibPrev = NULL;
                	return temp;
                }
}             

/*
FunctionName: outChild
Parameters: pointer to pcb to be removed, p
Return: pointer to removed pcb, p
Purpose: Orphan any given child, p. If p has no parent return NULL.
*/             
pcb_PTR outChild (pcb_PTR p) {
                /*case 1, no parent*/
                if (p -> p_prnt == NULL){
                                return NULL;
                }
                else if (p == NULL) {
                	return NULL;
                }                 
                else {
                /*case 2, has parent*/
                if ((p -> p_sibNext) == NULL) { /*no next sibling*/
                                if (p -> p_sibPrev != NULL) { /* previous siblings*/
                                                p -> p_sibPrev -> p_sibNext = NULL;
                                }
                                /*happens with both these cases*/
                                return p;                             
                }
                else {
                                if ((p -> p_sibPrev) == NULL) { /*next child, no previous, can also just call removeChild*/
                                                return removeChild(p -> p_prnt);
                                }
                                else {
                                /*middle child*/
                                p -> p_sibPrev -> p_sibNext = p -> p_sibNext;
                                p -> p_sibNext -> p_sibPrev = p -> p_sibPrev;
                                }
                		 return p;  
                }
                }
                return p;           
}             
               
/*
FunctionName: freePcb
Parameters: pcb pointer, p
Return: n/a
Purpose: send pcb that is no longer in use to the pcdFree list
*/
void freePcb (pcb_PTR p) { /*insert p into free list*/
                p -> p_next = pcb_freeH; /*make p the first thing in free list*/
                pcb_freeH = p; /*free list pointer points to p, so it's what's accessed in the list*/
}

/*
FunctionName: initPcbs
Parameters: none
Return: none
Purpose: Initialize pcbFree List that holds all of the elements in MAXPROC
*/ 
void initPcbs() {
                pcb_freeH = mkEmptyProcQ();
                static pcb_t new[MAXPROC];
                int i;
                for (i = 0; i < MAXPROC; i++) {
                                freePcb(&(new[i]));
                }
}
              
