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
#include "queue.h"

#define PROCESS_NUM 10					// �������μ����� ����
#define TICK_SEC 0						// �� ���� TICK ����
#define TICK_USEC 10					// ����ũ�� �� ���� TICK ����
#define OS_RUNNING_TIME 10000			// �� RUNNING TIME ����
#define TIME_QUANTUM 5					// 1QUANTUM ����
#define BURST_RANGE 10					// CPU burst ���� IO burst����  random �ϰ� ������ٶ�  % ���ִ� ����
#define KEY_NUM 0x3655					// Msg Queue Key ��

typedef struct msgbuf{
	int mtype;
	int io_time;
	pid_t pid;
}msgbuf;

void userAction();						// USER process�� �۵��ϴ� ����
void kernelHandler(int signo);			// Kernel handler
void userHandler(int signo);			// USER handler

struct QUEUE *readyQueue;				// ReadyQueue
struct QUEUE *waitQueue;				// WaitQueue
struct PCB *runQueue;					/* runQueue -> �����  1���� ���μ����� runQueue�� �����ϹǷ�
										   Queue�� ���°� �ƴ� process�� ������ ����ִ�  pcb�� ���·� ���� �Ǿ���. */

struct PCB *pcb[PROCESS_NUM];			// Kernel���� USER process�� IO burst�� pid�� �����ϴ� ��

int osRunningTime = 0;					// ���� Kernel�� ���� �ð�
int remainingCpuBurst;					// ���� CPUburst��
pid_t kernelPid;						// Kernel PID

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
