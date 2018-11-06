#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<signal.h>
#include<errno.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"queue.h"

#define TICK_TIME 2
#define BURST_RANGE 10
#define USER_PROCESS_NUM 10

void singleTick(int signo);
void cpuAction(int signo);
void afterAction(int signo);

PCB *scheduled_pcb;
Queue *runQueue;
pid_t kernelPID;

int main()
{
	int i = 0;
	int random[USER_PROCESS_NUM];
	pid_t pid[USER_PROCESS_NUM];
	struct sigaction kernelBeforeAction;
	struct sigaction oldkernelBeforeAction;
	struct sigaction kernelAfterAction;
	struct sigaction oldkernelAfterAction;

	runQueue = (Queue*)malloc(sizeof(Queue));
	scheduled_pcb = (PCB*)malloc(sizeof(PCB));
	runQueue = CreateQueue();
	//create run queue

	memset(&kernelBeforeAction, 0, sizeof(kernelBeforeAction));
	memset(&kernelAfterAction, 0, sizeof(kernelAfterAction));
	kernelBeforeAction.sa_handler = &singleTick;
	kernelAfterAction.sa_handler = &afterAction;
	sigaction(SIGALRM, &kernelBeforeAction, &oldkernelBeforeAction);
	sigaction(SIGUSR2, &kernelAfterAction, &oldkernelAfterAction);

	srand((int)time(NULL));
	kernelPID = getpid();
	printf("Kernel PID: %d\n", (int)kernelPID);

	for(i = 0; i < USER_PROCESS_NUM; i++){
		random[i] = (rand() % BURST_RANGE) + 1;
		pid[i] = fork();
		if(pid[i] == 0){
			struct sigaction userAction;
			struct sigaction oldUserAction;
			memset(&userAction, 0, sizeof(userAction));
			userAction.sa_handler = &cpuAction;
			sigaction(SIGUSR1, &userAction, &oldUserAction);
		}
		else if(pid[i] < 0){
		    perror("fork");
		    exit(0);
		}
	}

	PCB *temp_pcb = (PCB*)malloc(sizeof(PCB));

	for(i = 0; i < USER_PROCESS_NUM; i++)
	{
		printf("User Process: %d with CPU burst value of: %d\n", (int)pid[i], random[i]);
		temp_pcb->pid = pid[i];
		temp_pcb->CPU_burst = random[i];
		addprocess(runQueue, temp_pcb);
		//enqueue all random CPU burst value and PID
	}

	struct itimerval new_itimer, old_itimer;
	new_itimer.it_interval.tv_sec = TICK_TIME;
	new_itimer.it_interval.tv_usec = 0;
	new_itimer.it_value.tv_sec = TICK_TIME;
	new_itimer.it_value.tv_usec = 0;
	printf("\n*********Time Starts Now*********\n\n");
	setitimer(ITIMER_REAL, &new_itimer, &old_itimer);

	while(1);
}

void singleTick(int signo)
{
	scheduled_pcb = removeprocess(runQueue, runQueue->head->pcb);
	//dequeue run queue
	//save to scheduled process

	kill(scheduled_pcb->pid, SIGUSR1);
}


void cpuAction(int signo)
{
	scheduled_pcb -> CPU_burst--;
	printf("User Process: %d with CPU burst value of: %d\n", getpid(), scheduled_pcb -> CPU_burst);
	kill(kernelPID, SIGUSR2);
	if(scheduled_pcb -> CPU_burst ==  0)
	{
		printf("User Process: %d finished!\n", getpid());
		exit(0);
	}
}

void afterAction(int signo)
{
	if(scheduled_pcb -> CPU_burst != 0)
		addprocess(runQueue, scheduled_pcb);

	if(runQueue->count == 0 )
		printf("All Finished! Shutting Down\n");
		exit(0);
}
