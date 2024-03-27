#include "RTOS.h"

taskStack_t task1Env[TASK_STACK_SIZE];
taskStack_t task2Env[TASK_STACK_SIZE];
taskStack_t task3Env[TASK_STACK_SIZE];
taskStack_t task4Env[TASK_STACK_SIZE];

task_t ttask1;
task_t ttask2;
task_t ttask3;
task_t ttask4;

timer_t timer1, timer2, timer3;

uint32_t bit1, bit2, bit3;

void timerFuc(void* arg) {
	uint32_t* ptrBit = (uint32_t*)arg;
	*ptrBit ^= 0x1;
	
}

void delay() {
	for (uint32_t i = 0; i < 0xffff; ++i);
}

int task1Flag;
void task1Entry (void* param) {
	uint32_t stopped = 0;
	
	setSysTick(SYS_TICK);
	timerInit(&timer1, 100, 10, timerFuc, &bit1, TIMER_CONFIG_TYPE_STRICT);
	timerStart(&timer1);
	timerInit(&timer2, 200, 20, timerFuc, &bit2, TIMER_CONFIG_TYPE_STRICT);
	timerStart(&timer2);
	timerInit(&timer3, 300, 30, timerFuc, &bit3, TIMER_CONFIG_TYPE_LOOSE);
	timerStart(&timer3);
	while(1) {
		task1Flag = 0;
		taskDelay(10);
		task1Flag = 1;
		taskDelay(10);
		
		if (!stopped) {
			taskDelay(2000);
			timerDestory(&timer1);
		}
		
		timerInfo_t info = timerGetInfo(&timer2);
	}
}


int task2Flag;
void task2Entry (void* param) {
	while(1) {
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

