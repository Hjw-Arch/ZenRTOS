#include "RTOS.h"
#include "ARMCM3.h" 

listHead listhead;
listNode listnode[8];


int task1Flag;
void task1Entry (void* param) {
	setSysTick(SYS_TICK);
	
	while(1) {
		task1Flag = 0;
		taskDelay(20);
		task1Flag = 1;
		taskDelay(20);
	}
}

void delay() {
	for (int i = 0; i < 0xff; i++);
}

int task2Flag;
void task2Entry (void* param) {
	while(1) {
		task2Flag = 0;
		delay();
		task2Flag = 1;
		delay();
	}
}

int task3Flag;
void task3Entry (void* param) {
	while(1) {
		task3Flag = 0;
		delay();
		task3Flag = 1;
		delay();
	}
}

task_t ttask1;
task_t ttask2;
task_t ttask3;

taskStack_t task1Env[1024];
taskStack_t task2Env[1024];
taskStack_t task3Env[1024];

void rtosInit() {
	// idletask
	schedLockInit();   //û��Ҫ
	bitmapInit(&taskPriorityBitmap);   //û��Ҫ
	taskDelayedListInit();
	// ��ʼ����������
	for (int i = 0; i < RTOS_PRIORITY_COUNT; ++i) {
		listHeadInit(&taskTable[i]);
	}
	taskInit(&_idleTask, idleTaskEntry, (void*)0, &idleTaskEnv[512], RTOS_PRIORITY_COUNT - 1);
	idleTask = &_idleTask;
}

int main(){
	rtosInit();
	
	taskInit(&ttask1, task1Entry, (void*)0x1145, &task1Env[1024], 0);
	taskInit(&ttask2, task2Entry, (void*)0x1919, &task2Env[1024], 1);
	taskInit(&ttask3, task3Entry, (void*)0x1919, &task3Env[1024], 1);
	
	nextTask = getHighestReadyTask();
	
	runFirstTask2(); // ����֮�󲻻᷵�أ��·���return 0��ʵûʲô����
	
	return 0;
}

