#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<signal.h>
#include<sys/time.h>
#include<sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PROCESS_NUM 10
#define TICK_SEC 0
#define TICK_USEC 1000
#define OS_RUNNING_TIME 10000
#define TIME_QUANTUM 10
#define BURST_RANGE 5
#define KEY_NUM 0x3655

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

typedef struct msgbuf{
	int mtype;
	int io_time;
	pid_t pid;
}msgbuf;

QUEUE* createQueue();
PCB* dequeue(QUEUE *queue);
void enqueue(QUEUE *queue, PCB *pcb);
void priorityEnqueue(QUEUE *queue, PCB *pcb);
void userAction();
void kernelHandler(int signo);
void userHandler(int signo);

struct QUEUE *readyQueue;
struct QUEUE *waitQueue;
struct PCB *runQueue;

struct PCB *pcb[PROCESS_NUM];

int osRunningTime = 0;
int remainingCpuBurst;
pid_t kernelPid;

int main()
{
	int i;
	int runCount = 0;
	int ioCount = 0;
	pid_t pid[PROCESS_NUM];
	kernelPid = getpid();
	int msgq;
	int key = KEY_NUM;
	struct msgbuf msg;
	struct PCB *waitTemp;
	readyQueue = createQueue();
	waitQueue = createQueue();
	struct sigaction oldKernel, newKernel;
	printf("Kernel with %d\n", getpid());
	for (i = 0; i < PROCESS_NUM; i++)
	{
		pid[i] = fork();
		if (pid[i] == 0)
		{
			printf("User #%d with %d\n", i, getpid());
			srand((int)time(NULL) + i);
			userAction();
		}
		else if(pid[i] < 0)
		{
			printf("ERROR\n");
			exit(0);
		}
		pcb[i] = (PCB*)malloc(sizeof(PCB));
		memset(pcb[i], 0, sizeof(PCB));
		pcb[i]->pid = pid[i];
		pcb[i]->remainingIoBurst = 0;
		pcb[i]->ioBurst = 0;
		pcb[i]->timeQuantum = TIME_QUANTUM;
		enqueue(readyQueue, pcb[i]); //change
	}
	memset(&msg, 0, sizeof(msg));
	msgq = msgget( key, IPC_CREAT | 0666);
	memset(&newKernel, 0, sizeof(newKernel));
	newKernel.sa_handler = &kernelHandler;
	sigaction(SIGALRM, &newKernel, &oldKernel);
	runQueue = dequeue(readyQueue);
	runCount++;

	struct itimerval new_itimer, old_itimer;
	new_itimer.it_interval.tv_sec = TICK_SEC;
	new_itimer.it_interval.tv_usec = TICK_USEC;
	new_itimer.it_value.tv_sec = TICK_SEC;
	new_itimer.it_value.tv_usec = TICK_USEC;
	setitimer(ITIMER_REAL, &new_itimer, &old_itimer);

	while (osRunningTime < OS_RUNNING_TIME)
	{
		if(msgrcv(msgq, &msg, sizeof(msg), 0, IPC_NOWAIT) != -1)
		{
			printf("%d message recieved\n", msg.pid);
			if(runQueue->pid == msg.pid)
			{
				runQueue->ioBurst = msg.io_time;
				runQueue->timeQuantum = TIME_QUANTUM;
				priorityEnqueue(waitQueue, runQueue);
				runQueue = dequeue(readyQueue);
				runCount++;
			}
		}
		else if(runQueue->timeQuantum == 0)
		{
			runQueue->timeQuantum = TIME_QUANTUM;
			enqueue(readyQueue, runQueue);
			runQueue = dequeue(readyQueue);
			runCount++;
		}
		while(waitQueue->count != 0 && waitQueue->head->pcb->remainingIoBurst == 0)
		{
			waitTemp = dequeue(waitQueue);
			enqueue(readyQueue, waitTemp);
			ioCount++;
		}
	}
	for(i = 0; i < PROCESS_NUM; i++)
		kill(pid[i], SIGKILL);
	printf("OS Times Up! Ending Now\nTotal Run Queue Completion: %d\nTotal Wait Queue Completion: %d\n", runCount, ioCount);
	exit(0);
}

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

void userAction()
{
	int msgq;
	int ret;
	int key = KEY_NUM;
	msgq = msgget( key, IPC_CREAT | 0666);
	struct msgbuf msg;
	memset(&msg, 0, sizeof(msg));
	struct sigaction oldUser, newUser;
	int cpuBurst = (rand() % BURST_RANGE) + 1;
	int ioBurst = (rand() % BURST_RANGE) + 1;
	remainingCpuBurst = cpuBurst;
	memset(&newUser, 0, sizeof(newUser));
	newUser.sa_handler = &userHandler;
	sigaction(SIGUSR1, &newUser, &oldUser);
	while (1)
	{
		if(remainingCpuBurst <= 0)
		{
			msg.mtype = 0;
			msg.pid = getpid();
			msg.io_time = ioBurst;
			printf("%d message sent\n", msg.pid);
			ret = msgsnd(msgq, &msg, sizeof(msg), 0);
			remainingCpuBurst = cpuBurst;
		}
	}
}

void kernelHandler(int signo)
{
	osRunningTime++;
	runQueue->timeQuantum--;
	if(waitQueue->count != 0)
		waitQueue->head->pcb->remainingIoBurst--;
	printf("Run pid: %d\nCPU time: %d\nTime Quantum: %d\n", runQueue->pid, osRunningTime, runQueue->timeQuantum);
	kill(runQueue->pid, SIGUSR1);
	if(waitQueue->count != 0)
		printf("wait pid: %d\nRemaining IO burst time %d\n\n", waitQueue->head->pcb->pid, waitQueue->head->pcb->remainingIoBurst);
}

void userHandler(int signo)
{
	remainingCpuBurst--;
	printf("CPU Burst: %d\n---------------------------\n", remainingCpuBurst);
}
