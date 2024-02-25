#include "RTOS.h"

taskStack_t task1Env[TASK_STACK_SIZE];
taskStack_t task2Env[TASK_STACK_SIZE];
taskStack_t task3Env[TASK_STACK_SIZE];

task_t ttask1;
task_t ttask2;
task_t ttask3;

int task1Flag;
void task1Entry (void* param) {
	setSysTick(SYS_TICK);
	
	while(1) {
		task1Flag = 0;
		taskSuspend(currentTask);
		task1Flag = 1;
		taskSuspend(currentTask);
	}
}

void delay() {
	for (int i = 0; i < 0xff; i++);
}

int task2Flag;
void task2Entry (void* param) {
	while(1) {
		task2Flag = 0;
		taskDelay(10);
		taskWakeUp(&ttask1);
		task2Flag = 1;
		taskDelay(10);
		taskWakeUp(&ttask1);
	}
}

int task3Flag;
void task3Entry (void* param) {
	while(1) {
		task3Flag = 0;
		taskDelay(10);
		task3Flag = 1;
		taskDelay(10);
	}
}

void appInit() {
	taskInit(&ttask1, task1Entry, (void*)0x1145, &task1Env[1024], 0);
	taskInit(&ttask2, task2Entry, (void*)0x1919, &task2Env[1024], 1);
	taskInit(&ttask3, task3Entry, (void*)0x1919, &task3Env[1024], 1);
}

