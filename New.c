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

#define CPU_TIME_QUANTUM 3
#define USER_PROCESS_NUM 5

void parent_handler(int signo);
void end_handler(int signo);
void child_action();
void child_handler(int signo);
void io_action();

struct PCB *pcb[10];		// For All pcb
struct PCB *present_pcb;	// For present job
struct Queue *readyQueue;		// main queue

int remain_cpu_burst;
int parentPID;
int end_flag = 0;
int end_count = 0;

int main()
{
	pid_t pid;
	int i;

	struct sigaction old_sa, new_sa;
	struct sigaction old_endtimer, new_endtimer;
	struct itimerval new_itimer, old_itimer;

	parentPID = getpid();
	readyQueue = (Queue*)malloc(sizeof(Queue));
	readyQueue = CreateQueue();

	for(int i = 0; i < USER_PROCESS_NUM ; i++) {
		pid = fork();
		if (pid < -1) {
			perror("fork error");
			return 0;
			}
		else if (pid == 0) {
			// child
			printf("child process with pid %d\n", getpid());
			srand((int)time(NULL)+i);
			child_action();
			}
		else {
			// parent
			printf("my pid is %d\n", getpid());
			pcb[i] = (PCB*)malloc(sizeof(PCB));
			memset(pcb[i], 0, sizeof(PCB));
			pcb[i]->pid = pid;
			pcb[i]->remain_CPU_TIME_QUANTUM = CPU_TIME_QUANTUM;
			addprocess(readyQueue, pcb[i]);
			}
		}

	memset(&new_sa, 0, sizeof(new_sa));
	memset(&new_endtimer, 0, sizeof(new_endtimer));
	new_sa.sa_handler = &parent_handler;
	new_endtimer.sa_handler = &end_handler;
	sigaction(SIGALRM, &new_sa, &old_sa);
	sigaction(SIGUSR2, &new_endtimer, &old_endtimer);

	//timer setting
	new_itimer.it_interval.tv_sec = 1;
	new_itimer.it_interval.tv_usec = 0;
	new_itimer.it_value.tv_sec = 1;
	new_itimer.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &new_itimer, &old_itimer);

	while(end_count < USER_PROCESS_NUM);
	exit(0);
}

void parent_handler(int signo){
	if(present_pcb == NULL)
		present_pcb = removeprocess(readyQueue, readyQueue->head->pcb);

	present_pcb -> remain_CPU_TIME_QUANTUM--;
	kill(present_pcb -> pid, SIGUSR1);

	if(end_flag == 0)
	{
		if(present_pcb -> remain_CPU_TIME_QUANTUM == 0)
		{
			present_pcb -> remain_CPU_TIME_QUANTUM = CPU_TIME_QUANTUM;
			addprocess(readyQueue, present_pcb);
			present_pcb = removeprocess(readyQueue, readyQueue->head->pcb);
			end_flag = 0;
		}
	}
	else if(readyQueue->count != 0)
	{
		present_pcb = removeprocess(readyQueue, readyQueue->head->pcb);
		end_flag = 0;

	}
	else
		end_flag = 0;
}

void end_handler(int signo){
	end_flag = 1;
	end_count++;
}

void child_action()
{
	struct sigaction oldchild_sa, newchild_sa;
	remain_cpu_burst = (rand() % 5) + 1;

	memset(&newchild_sa, 0, sizeof(newchild_sa));
	newchild_sa.sa_handler = &child_handler;
	sigaction(SIGUSR1, &newchild_sa, &oldchild_sa);
	while(1);
}

void child_handler(int signo){
	printf("Proc (%d) remaining cpu time is : %d\n", getpid(), remain_cpu_burst);
	remain_cpu_burst--;
	if(remain_cpu_burst == 0){
		io_action();
	}
}

void io_action(){

}








