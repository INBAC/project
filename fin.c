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
#include<sys/ipc.h>
#include<sys/msg.h>
#include"queue.h"

#define CPU_TIME_QUANTUM 5
#define USER_PROCESS_NUM 10
#define MSG_KEY 0xa3214

void parent_handler(int signo);
void child_action(int cpu_burst);
void child_handler(int signo);
void io_action();

struct PCB *pcb[10];		// For All pcb
struct PCB *present_pcb;	// For present job
struct Queue *readyQueue;		// main queue
struct Queue *waitQueue;		// wait queue

int remain_cpu_burst;
int cpu_burst[USER_PROCESS_NUM];
int globaltime = 0;

int main()
{
	pid_t pid;
	int i;

	struct sigaction old_sa, new_sa;
	struct itimerval new_itimer, old_itimer;

	readyQueue = (Queue*)malloc(sizeof(Queue));
	waitQueue = (Queue*)malloc(sizeof(Queue));
	readyQueue = CreateQueue();
	waitQueue = CreateQueue();

	srand((int)time(NULL));
	for(int i = 0; i < USER_PROCESS_NUM ; i++) {
		cpu_burst[i] = (rand() % 10) + 1;
		globaltime = globaltime + cpu_burst[i];
		pid = fork();
		if (pid < -1) {
			perror("fork error");
			return 0;
			}
		else if (pid == 0) {
			// child
			printf("child process with pid %d\n", getpid());
			child_action(cpu_burst[i]);
			}
		else {
			// parent
			printf("my pid is %d\n", getpid());
			pcb[i] = (PCB*)malloc(sizeof(PCB));
			memset(pcb[i], 0, sizeof(PCB));
			pcb[i]->pid = pid;
			pcb[i]->remain_CPU_burst = cpu_burst[i];
			pcb[i]->remain_CPU_TIME_QUANTUM = CPU_TIME_QUANTUM;
			addprocess(readyQueue, pcb[i]);
			}
		}

	memset(&new_sa, 0, sizeof(new_sa));
	new_sa.sa_handler = &parent_handler;
	sigaction(SIGALRM, &new_sa, &old_sa);

	//timer setting
	new_itimer.it_interval.tv_sec = 1;
	new_itimer.it_interval.tv_usec = 0;
	new_itimer.it_value.tv_sec = 1;
	new_itimer.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &new_itimer, &old_itimer);

	while(globaltime > 0){
		///// Child에게 IPC 메세지를 수신하고 child의 I/O burst 시작여부 판단
		///// 스케줄러는 해당 child를 readyQueue에서 remove하고 그 child를 waitQueue로 이동
		///// 이때 받은 I/O burst 값을 parent는 기억하고 있어야 한다.
		///// 1 TICK 마다 I/O burst value를 1씩 줄인다. (당연히 모든 waitQueue에 있는것들을)
		///// 0이 되면 다시 이것을 runQueue로 가져 온다.
		///// 참고로 총 실행(전체 실행 조건)은 TimeTick의 횟수로 정해준다.
	};
	exit(0);
}

void parent_handler(int signo){
	globaltime--;
	if(present_pcb == NULL)
		present_pcb = removeprocess(readyQueue, readyQueue->head->pcb);

	present_pcb -> remain_CPU_TIME_QUANTUM--;
	present_pcb -> remain_CPU_burst--;
	kill(present_pcb -> pid, SIGUSR1);
	if(present_pcb -> remain_CPU_burst != 0)
	{
		if(present_pcb -> remain_CPU_TIME_QUANTUM == 0)
		{
			present_pcb -> remain_CPU_TIME_QUANTUM = CPU_TIME_QUANTUM;
			addprocess(readyQueue, present_pcb);
			present_pcb = removeprocess(readyQueue, readyQueue->head->pcb);
		}
	}
	else if(readyQueue->count != 0)
	{
		present_pcb = removeprocess(readyQueue, readyQueue->head->pcb);
	}

}

void child_action(int cpu_burst)
{
	struct sigaction oldchild_sa, newchild_sa;
	remain_cpu_burst = cpu_burst;

	memset(&newchild_sa, 0, sizeof(newchild_sa));
	newchild_sa.sa_handler = &child_handler;
	sigaction(SIGUSR1, &newchild_sa, &oldchild_sa);

	while(remain_cpu_burst > 0);
	printf("PID %d EXIT\n", getpid());
	exit(0);
}

void child_handler(int signo){
	printf("Proc (%d) remaining cpu time is : %d\n", getpid(), remain_cpu_burst);
	remain_cpu_burst--;
	if(remain_cpu_burst == 0){
		io_action();
	}
}

void io_action()
{
	int msgq;
	int ret;

}



