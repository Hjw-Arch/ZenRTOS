#include "RTOS.h"
#include "ARMCM3.h"


int task1Flag;
void task1Entry (void* param) {
	setSysTick(TIME_SLICE);
	while(1) {
		task1Flag = 0;
		taskDelay(20);
		task1Flag = 1;
		taskDelay(20);
	}
}

int task2Flag;
void task2Entry (void* param) {
	while(1) {
		task2Flag = 0;
		taskDelay(20);
		task2Flag = 1;
		taskDelay(20);
	}
}

task_t ttask1;
task_t ttask2;

taskStack_t task1Env[1024];
taskStack_t task2Env[1024];

void rtosInit() {
	// idletask
	schedLockInit();   //没必要
	bitmapInit(&taskPriorityBitmap);   //没必要
	taskInit(&_idleTask, idleTaskEntry, (void*)0, &idleTaskEnv[512], RTOS_PRIORITY_COUNT - 1);
	idleTask = &_idleTask;
}

int main(){
	rtosInit();
	
	taskInit(&ttask1, task1Entry, (void*)0x1145, &task1Env[1024], 0);
	taskInit(&ttask2, task2Entry, (void*)0x1919, &task2Env[1024], 1);
	
	taskTable[0] = &ttask1;
	taskTable[1] = &ttask2;
	
	nextTask = getHighestReady();
	
	runFirstTask2(); // 运行之后不会返回，下方的return 0其实没什么作用
	
	return 0;
}

