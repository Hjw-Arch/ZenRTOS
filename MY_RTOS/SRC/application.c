#include "RTOS.h"

taskStack_t task1Env[TASK_STACK_SIZE];
taskStack_t task2Env[TASK_STACK_SIZE];
taskStack_t task3Env[TASK_STACK_SIZE];
taskStack_t task4Env[TASK_STACK_SIZE];

task_t ttask1;
task_t ttask2;
task_t ttask3;
task_t ttask4;

sem_t semaphore1, semaphore2;


int task1Flag;

void task1Entry (void* param) {
	setSysTick(SYS_TICK);
	
	semInit(&semaphore1, 0, 10);
	semInit(&semaphore2, 0, 0);
	
	while(1) {
		semWait(&semaphore1, 0);
		task1Flag = 0;
		taskDelay(10);
		task1Flag = 1;
		taskDelay(10);
	}
}

int task2Flag;
void task2Entry (void* param) {
	int error = 0;
	while(1) {
		task2Flag = 0;
		taskDelay(10);
		task2Flag = 1;
		taskDelay(10);
		
		semPost(&semaphore1);
		
		error = semGetWithNoWait(&semaphore1);
	}
}

int task3Flag;
void task3Entry (void* param) {
	while(1) {
		semWait(&semaphore2, 100);
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

