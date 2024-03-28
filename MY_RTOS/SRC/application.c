#include "RTOS.h"

taskStack_t task1Env[TASK_STACK_SIZE];
taskStack_t task2Env[TASK_STACK_SIZE];
taskStack_t task3Env[TASK_STACK_SIZE];
taskStack_t task4Env[TASK_STACK_SIZE];

task_t ttask1;
task_t ttask2;
task_t ttask3;
task_t ttask4;

taskInfo_t taskinfo1, taskinfo2, taskinfo3, taskinfo4;

int task1Flag;
void task1Entry (void* param) {
	while(1) {
		task1Flag = 0;
		taskDelay(10);
		task1Flag = 1;
		taskDelay(10);
		
		taskinfo1 = taskGetInfo(&ttask1);
	}
}


int task2Flag;
void task2Entry (void* param) {
	while(1) {
		task2Flag = 0;
		taskDelay(10);
		task2Flag = 1;
		taskDelay(10);
		
		taskinfo2 = taskGetInfo(&ttask2);
	}
}

int task3Flag;
void task3Entry (void* param) {
	while(1) {
		task3Flag = 0;
		taskDelay(10);
		task3Flag = 1;
		taskDelay(10);
		
		taskinfo3 = taskGetInfo(&ttask3);
	}
}

int task4Flag;
void task4Entry (void* param) {
	while(1) {
		task4Flag = 0;
		taskDelay(10);
		task4Flag = 1;
		taskDelay(10);
		
		taskinfo4 = taskGetInfo(&ttask4);
	}
}

void appInit() {
	taskInit(&ttask1, task1Entry, (void*)0x1145, task1Env, 0, sizeof(task1Env));
	taskInit(&ttask2, task2Entry, (void*)0x1919, task2Env, 1, sizeof(task2Env));
	taskInit(&ttask3, task3Entry, (void*)0x1919, task3Env, 1, sizeof(task3Env));
	taskInit(&ttask4, task4Entry, (void*)0x1919, task4Env, 1, sizeof(task4Env));
}

