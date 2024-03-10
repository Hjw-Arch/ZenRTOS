#include "RTOS.h"

taskStack_t task1Env[TASK_STACK_SIZE];
taskStack_t task2Env[TASK_STACK_SIZE];
taskStack_t task3Env[TASK_STACK_SIZE];
taskStack_t task4Env[TASK_STACK_SIZE];

task_t ttask1;
task_t ttask2;
task_t ttask3;
task_t ttask4;

mbox_t box1, box2;

#define MAILBOX_SIZE 16

void* messageBuffer1[MAILBOX_SIZE];
void* messageBuffer2[MAILBOX_SIZE];

uint32_t msg1[MAILBOX_SIZE];
uint32_t msg2[MAILBOX_SIZE];

mboxInfo_t info;

int task1Flag;
void task1Entry (void* param) {
	setSysTick(SYS_TICK);
	mboxInit(&box1, messageBuffer1, MAILBOX_SIZE);
	mboxInit(&box2, messageBuffer2, MAILBOX_SIZE);
	void* msg;
	
	info = mboxGetInfo(&box1);
	mboxWait(&box1, &msg, 0);
	info = mboxGetInfo(&box1);
	task1Flag = *(uint32_t*)msg;
	while(1) {
		task1Flag = 0;
		taskDelay(10);
		task1Flag = 1;
		taskDelay(10);
	}
}


int task2Flag;
void task2Entry (void* param) {
	while(1) {
		void* msg = (void*)0x3f;
		info = mboxGetInfo(&box1);
		mboxPost(&box1, &msg, PRIORITY_NORMAL);
		info = mboxGetInfo(&box1);
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

