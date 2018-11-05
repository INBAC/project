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

#define TICK_TIME 50
#define USER_PROCESS_NUM 10



void main()
{
	int i = 0;
	int CPUburst[USER_PROCESS_NUM];
	pid_t pid[USER_PROCESS_NUM];

	srand((int)time(NULL));
	for(i = 0; i < USER_PROCESS_NUM; i++)
	{
		pid[i] = fork();
		if(pid[i] == 0)
		{
			exit(0);
		}
		else if(pid[i] < 0)
		{
		    perror("fork");
		    abort();
		}
	}

}
