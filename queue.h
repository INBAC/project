#include<stdio.h>
#include<stdlib.h>

typedef struct {
	int pid;
	int CPU_burst;
} PCB;

typedef struct PROCESS{
		PCB pcb;
		struct PROCESS *next;
} PROCESS;

typedef struct {
	int count;
	PROCESS *head;
	PROCESS *pos;
} Queue;

Queue *CreateQueue(){
	Queue *NewQueue = (Queue*)malloc(sizeof(Queue));
	NewQueue->pos = NULL;
	NewQueue->head = NULL; //initialize
	NewQueue->count = 0;
	return NewQueue;
}

int SearchQueue(Queue *p, PROCESS **ppPre, PROCESS **ppLoc, PCB *pcb){
	for(*ppPre = NULL, *ppLoc = p->head; *ppLoc != NULL; *ppPre = *ppLoc, *ppLoc = (*ppLoc)->next){
		if((*ppLoc)->pcb->pid == pcb->pid)
			return TRUE;
		else if((*ppLoc)->pcb->pid > pcb->pid)
			break;
	}
	return FALSE;
}

void insertQueue(Queue *p, PROCESS *pPre, PCB *pcb){
	PROCESS *New_process = (PROCESS*)malloc(sizeof(PROCESS));
	New_process->pcb = pcb;

	if(pPre == NULL){
		New_process->next = p->head;
		p->head = New_process;
	}
	else{
		New_process->next = pPre->next;
		pPre->next = New_process;
	}
	p->count++;
}

pcb *deleteQueue(Queue *p, PROCESS *pPre, PROCESS *pLoc){
	PROCESS *temp = NULL;
	temp = pLoc->pcb;

	if(pPre == NULL)
		p->head = pLoc->next;
	else
		pPre->next = pLoc->next;

	free(pLoc);
	p->count--;
	return temp;
}

void addprocess(Queue *p, PCB *pcb){
	PROCESS *pPre = NULL, *pLoc = NULL;
	int found;
	found = SearchQueue(p, &pPre, &pLoc, pcb);
	if(!found)
		insertQueue(p, pPre, pcb);
}

pcb* removeprocess(Queue *p, PCB *pcb){
	PROCESS *pPre = NULL, *pLoc = NULL;
	pcb *temp = NULL;
	int found;

	found = SearchQueue(p, &pPre, &pLoc, pcb);
	if(found)
		temp = deleteQueue(p, pPre, pLoc);
	return temp;
}

void destroyprocess(Queue *p){
	PROCESS *pDel = NULL, *pNext = NULL;
	for(pDel = p->head; pDel != NULL; pDel = pNext){
		pNext = pDel->next;
		free(pDel);
	}
	free(p);
}
