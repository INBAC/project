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

#define TICK_TIME 2
#define BURST_RANGE 10
#define USER_PROCESS_NUM 10

void singleTick(int signo);
void cpuAction(int signo);
void afterAction(int signo);

//scheduled process structure
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

	for(i = 0; i < USER_PROCESS_NUM; i++)
	{
		random[i] = (rand() % BURST_RANGE) + 1;
		pid[i] = fork();
		if(pid[i] == 0)
		{
			struct sigaction userAction;
			struct sigaction oldUserAction;
			memset(&userAction, 0, sizeof(userAction));
			userAction.sa_handler = &cpuAction;
			sigaction(SIGUSR1, &userAction, &oldUserAction)
		}
		else it(pid[i] < 0)
		{
		    perror("fork");
		    abort();
		}
	}

	for(i = 0; i < USER_PROCESS_NUM; i++)
	{
		printf("User Process: %d with CPU burst value of: %d\n", (int)pid[i], random[i]);
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
	//dequeue run queue
	//save to scheduled process
	kill(/*scheduled pid*/, SIGUSR1);
}


void cpuAction(int signo)
{
	/*scheduled burst value*/--;
	kill(kernelPID, SIGUSR2);
	if(/*scheduled burst value*/ ==  0)
		exit(0);
}

void afterAction(int signo)
{
	if(/*scheduled burst value*/ != 0)
		//enqueue scheduled burst value and scheduled pid

	if(/*if queue is empty*/)
		exit(0);
}
