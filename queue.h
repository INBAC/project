#include<stdio.h>
#include<stdlib.h>

typedef struct PCB{
	int pid;
	int ioBurst;
	int remainingIoBurst;
	int timeQuantum;
}PCB;

typedef struct NODE{
	PCB *pcb;
	struct NODE *next;
}NODE;

typedef struct QUEUE{
	int count;
	NODE *head;
	NODE *tail;
}QUEUE;


QUEUE* createQueue()
{
	QUEUE *newQueue = (QUEUE*)malloc(sizeof(QUEUE));
	newQueue->count = 0;
	newQueue->head = NULL;
	newQueue->tail = NULL;
	return newQueue;
}

PCB* dequeue(QUEUE *queue)
{
	if(queue->count != 0)
	{
		NODE *nodeTemp = queue->head;
		PCB *pcbTemp = queue->head->pcb;
		if(queue->count == 1)
		{
			queue->head = NULL;
			queue->tail = NULL;
			queue->count--;
		}
		else
		{
			queue->head = queue->head->next;
			queue->count--;
			nodeTemp->next = NULL;
		}
		free(nodeTemp);
		return pcbTemp;
	}
	else
		return NULL;
}

void enqueue(QUEUE *queue, PCB *pcb)
{
	NODE *newNode = (NODE*)malloc(sizeof(NODE));
	newNode->pcb = pcb;
	newNode->next = NULL;
	if(queue->count == 0)
	{
		queue->head = newNode;
		queue->tail = newNode;
		queue->count++;
	}
	else
	{
		queue->tail->next = newNode;
		queue->tail = queue->tail->next;
		queue->count++;
	}

}

void priorityEnqueue(QUEUE *queue, PCB *pcb)
{
	if(queue->count == 0)
	{
		pcb->remainingIoBurst = pcb->ioBurst;
		enqueue(queue, pcb);
		return;
	}
	int firstRun = 1;
	NODE *past = NULL;
	NODE *future = NULL;
	NODE *present = (NODE*)malloc(sizeof(NODE));
	present->pcb = pcb;
	present->next = NULL;
	for(future = queue->head, past = NULL; future != NULL; past = future, future = future->next)
	{
		if(present->pcb->ioBurst <= future->pcb->ioBurst)
		{
			if(firstRun == 1)
			{
				present->pcb->remainingIoBurst = present->pcb->ioBurst;
				queue->head->pcb->remainingIoBurst = queue->head->pcb->ioBurst - present->pcb->ioBurst;
				present->next = queue->head;
				queue->head = present;
				queue->count++;
				return;
			}
			else
			{
				present->pcb->remainingIoBurst = present->pcb->ioBurst - past->pcb->ioBurst;
				future->pcb->remainingIoBurst = future->pcb->ioBurst - present->pcb->ioBurst;
				past->next = present;
				present->next = future;
				queue->count++;
				return;
			}
		}
		firstRun = 0;
	}
	pcb->remainingIoBurst = pcb->ioBurst - past->pcb->ioBurst;
	enqueue(queue, pcb);
	return;
}
