#include "RTOS.h"

taskStack_t task1Env[TASK_STACK_SIZE];
taskStack_t task2Env[TASK_STACK_SIZE];
taskStack_t task3Env[TASK_STACK_SIZE];
taskStack_t task4Env[TASK_STACK_SIZE];

task_t ttask1;
task_t ttask2;
task_t ttask3;
task_t ttask4;

eFlagGroup_t eflaggroup;

int task1Flag;
void task1Entry (void* param) {
	setSysTick(SYS_TICK);
	eFlagGroupInit(&eflaggroup, 0xFF);
	while(1) {
		task1Flag = 0;
		taskDelay(10);
		task1Flag = 1;
		taskDelay(10);
		
		eFlagGroupPost(&eflaggroup, 0x4, 0);
	}
}


int task2Flag;
void task2Entry (void* param) {
	uint32_t resultFlag;
	
	while(1) {
		eFlagGroupWait(&eflaggroup, EFLAGGROUP_ANY_RET | EFLAGGROUP_CLEAR_AFTER, 0x6, &resultFlag, 0);
		eFlagGroupGetWithNoWait(&eflaggroup, EFLAGGROUP_ALL_RET, 0x3, &resultFlag);
		task2Flag = 0;
		taskDelay(10);
		task2Flag = 1;
		taskDelay(10);
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

int task4Flag;
void task4Entry (void* param) {
	while(1) {
		task4Flag = 0;
		taskDelay(10);
		task4Flag = 1;
		taskDelay(10);
	}
}

void appInit() {
	taskInit(&ttask1, task1Entry, (void*)0x1145, &task1Env[TASK_STACK_SIZE], 0);
	taskInit(&ttask2, task2Entry, (void*)0x1919, &task2Env[TASK_STACK_SIZE], 1);
	taskInit(&ttask3, task3Entry, (void*)0x1919, &task3Env[TASK_STACK_SIZE], 1);
	taskInit(&ttask4, task4Entry, (void*)0x1919, &task4Env[TASK_STACK_SIZE], 1);
}

