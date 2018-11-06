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

#define TICK_TIME 500000
#define USER_PROCESS_NUM 10

void singleTick(int signo);
void cpuAction(int signo);

int main()
{
	int i = 0;
	pid_t pid[USER_PROCESS_NUM];
	struct sigaction oldKernalAction;
	struct sigaction kernalAction;

	srand((int)time(NULL));
	memset(&kernalAction, 0, sizeof(kernalAction));
	kernalAction.sa_handler = &singleTick;
	sigaction(SIGALRM, &kernalAction, &oldKernalAction);

	for(i = 0; i < USER_PROCESS_NUM; i++)
	{
		pid[i] = fork();
		if(pid[i] == 0)
		{

		}
		else it(pid[i] < 0)
		{
		    perror("fork");
		    abort();
		}
	}

	struct itimerval new_itimer, old_itimer;
	new_itimer.it_interval.tv_sec = 0;
	new_itimer.it_interval.tv_usec = TICK_TIME;
	new_itimer.it_value.tv_sec = 0;
	new_itimer.it_value.tv_usec = TICK_TIME;
	setitimer(ITIMER_REAL, &new_itimer, &old_itimer);
}

void singleTick(int signo)
{

}


void cpuAction(int signo)
{

}
