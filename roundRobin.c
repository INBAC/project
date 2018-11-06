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
#define USER_PROCESS_NUM 10

void singleTick(int signo);
void cpuAction(int signo);
void afterAction(int signo);

//global struct with selected pid and CPU burst value
pid_t kernalPID;

int main()
{
	int i = 0;
	int random[USER_PROCESS_NUM];
	pid_t pid[USER_PROCESS_NUM];
	struct sigaction kernalBeforeAction;
	struct sigaction oldKernalBeforeAction;
	struct sigaction kernalAfterAction;
	struct sigaction oldKernalAfterAction;

	srand((int)time(NULL));
	memset(&kernalBeforeAction, 0, sizeof(kernalBeforeAction));
	memset(&kernalAfterAction, 0, sizeof(kernalAfterAction));
	kernalBeforeAction.sa_handler = &singleTick;
	kernalAfterAction.sa_handler = &afterAction;
	sigaction(SIGALRM, &kernalBeforeAction, &oldKernalBeforeAction);
	sigaction(SIGUSR2, &kernalAfterAction, &oldKernalAfterAction);

	kernalPID = getpid();

	for(i = 0; i < USER_PROCESS_NUM; i++)
	{
		random[i] = (rand() % 10) + 1;
		pid[i] = fork();
		if(pid[i] == 0)
		{
			struct sigaction oldUserAction;
			struct sigaction userAction;
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

	//create queue and enqueue all random CPU burst and PID

	struct itimerval new_itimer, old_itimer;
	new_itimer.it_interval.tv_sec = TICK_TIME;
	new_itimer.it_interval.tv_usec = 0;
	new_itimer.it_value.tv_sec = TICK_TIME;
	new_itimer.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &new_itimer, &old_itimer);

	while(1);
}

void singleTick(int signo)
{
	//dequeue from run queue and save to global struct with selected pid and CPU burst value (line 19)
	//kill(selected PID, SIGUSR1);
}


void cpuAction(int signo)
{
	//decrement selected CPU burst
	//if CPU burst is exit(0)
	//kill(kernalPID, SIGUSR2);
}

void afterAction(int signo)
{
	//if selected CPU burst is not 0
		//enqueue the decremented user process and the selected PID

	//if run queue is empty
		//kill the parent process
}
