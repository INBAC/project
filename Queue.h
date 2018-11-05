#include<stdio.h>
#include<stdlib.h>

typedef struct {
	int pid;
	int CPU_burst;
} ITEM;

typedef struct PROCESS{
		ITEM item;
		struct PROCESS *next;
} PROCESS;

typedef struct {
	int count;
	PROCESS *front, *rear;
} Queue;

Queue *CreateQueue(){
	Queue NewQueue = (Queue*)malloc(sizeof(Queue));
	if(NewQueue == NULL)
			return NULL;

	NewQueue->count = 0;
	NewQueue->front = NewQueue->rear = NULL:

	return NewQueue;
}

void Enqueue(Queue *pQueue, ITEM item){
	PROCESS *Newptr = (PROCESS*)malloc(sizeof(PROCESS));
	if(Newptr == NULL)
		return;

	Newptr->item = item;
	Newptr->next = NULL;

	if(pQueue->front == 0){
		pQueue->front = pQueue->rear = Newptr;
	}
	else {
		pQueue->rear->next = Newptr;
		pQueue->rear = Newptr;
	}

	pQueue->count++;
}

ITEM Dequeue(Queue *pQueue){
	PROCESS *deleteLoc = NULL;
	ITEM item;

	if(pQueue->count == 0)
		return 0;

	deleteLoc = pQueue->front;
	item = deleteLoc->item;

	if(pQueue->count ==1){
		pQueue->front = pQueue->rear = NULL;
	}
	else {
		pQueue->front = deleteLoc->item;
	}

	free(deleteLoc);
	pQueue->count--;

	return item;
}

void DestroyQueue(Queue *pQueue){
	PROCESS *pCur = NULL, *pNext = NULL;

	for(pCur = pQueue->front; pCur!=NULL; pCur=pNext){
		pNext = pCur->next;
		free(pCur);
	}
	pQueue->count = 0;
	pQueue->front = pQueue->rear = NULL;

	free(pQueue);
}



