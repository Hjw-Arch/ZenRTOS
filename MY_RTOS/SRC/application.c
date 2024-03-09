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

uint32_t msg[16];

int task1Flag;
void task1Entry (void* param) {
	setSysTick(SYS_TICK);
	mboxInit(&box1, messageBuffer1, MAILBOX_SIZE);
	mboxInit(&box2, messageBuffer2, MAILBOX_SIZE);
	while(1) {
		for (int i = 0; i < MAILBOX_SIZE; ++i) {
			msg[i] = i;
			mboxPost(&box1, &msg[i], PRIORITY_NORMAL);
		}
		
		taskDelay(500);
		
		for (int i = 0; i < MAILBOX_SIZE; ++i) {
			msg[i] = i;
			mboxPost(&box1, &msg[i], PRIORITY_HIGH);
		}
		
		taskDelay(500);
		
		task1Flag = 0;
		taskDelay(10);
		task1Flag = 1;
		taskDelay(10);
	}
}


int task2Flag;
void task2Entry (void* param) {
	while(1) {
		void* msg;
		
		uint32_t err = mboxWait(&box1, &msg, 0);
		if (err == NO_ERROR) {
			int val = *(int *)msg;
			task2Flag = val;
			taskDelay(10);
		}
		
		task2Flag = 0;
		taskDelay(10);
		task2Flag = 1;
		taskDelay(10);
	}
}

int task3Flag;
void task3Entry (void* param) {
	while(1) {
		void* msg;
		mboxWait(&box2, &msg, 100);
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

