#include "RTOS.h"

taskStack_t task1Env[TASK_STACK_SIZE];
taskStack_t task2Env[TASK_STACK_SIZE];
taskStack_t task3Env[TASK_STACK_SIZE];
taskStack_t task4Env[TASK_STACK_SIZE];

task_t ttask1;
task_t ttask2;
task_t ttask3;
task_t ttask4;

eventCtrlBlock_t event1, event2;


int task1Flag;

void task1Entry (void* param) {
	setSysTick(SYS_TICK);
	
	eventInit(&event1, eventTypeUnknown);
	eventInit(&event2, eventTypeUnknown);
	
	while(1) {
		task1Flag = 0;
		eventWait(&event1, currentTask, NULL, 0, 50);
		taskSched();
		taskDelay(10);
		task1Flag = 1;
		taskDelay(10);
	}
}

void delay() {
	for (int i = 0; i < 0xff; i++);
}

int task2Flag;
void task2Entry (void* param) {
	while(1) {
		eventWait(&event2, currentTask, NULL, 0, 0);
		taskSched();
		task2Flag = 0;
		taskDelay(10);
		task2Flag = 1;
		taskDelay(10);
	}
}

int task3Flag;
void task3Entry (void* param) {
	while(1) {
		eventWait(&event2, currentTask, NULL, 0, 0);
		taskSched();
		task3Flag = 0;
		taskDelay(10);
		task3Flag = 1;
		taskDelay(10);
	}
}

int task4Flag;
void task4Entry (void* param) {
	while(1) {
		task_t* task = eventWakeUp(&event2, NULL, NO_ERROR);
		taskSched();
		task4Flag = 0;
		taskDelay(10);
		task4Flag = 1;
		taskDelay(10);
	}
}

void appInit() {
	taskInit(&ttask1, task1Entry, (void*)0x1145, &task1Env[TASK_STACK_SIZE], 0);
	taskInit(&ttask2, task2Entry, (void*)0x1919, &task2Env[TASK_STACK_SIZE], 1);
	taskInit(&ttask3, task3Entry, (void*)0x1919, &task3Env[TASK_STACK_SIZE], 0);
	taskInit(&ttask4, task4Entry, (void*)0x1919, &task4Env[TASK_STACK_SIZE], 1);
}

