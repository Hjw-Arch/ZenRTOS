#include "RTOS.h"

void rtosInit() {
	// idletask
	schedLockInit();   //û��Ҫ
	bitmapInit(&taskPriorityBitmap);   //û��Ҫ
	taskDelayedListInit();
	// ��ʼ����������
	for (int i = 0; i < RTOS_PRIORITY_COUNT; ++i) {
		listHeadInit(&taskTable[i]);
	}
	
	// ��ʼ����������
	taskInit(&_idleTask, idleTaskEntry, (void*)0, &idleTaskEnv[512], RTOS_PRIORITY_COUNT - 1);
	idleTask = &_idleTask;
	
	// ������ʱ������
	timerFuncInit();
}

int main(){
	rtosInit();
	
	appInit();
	
	nextTask = getHighestReadyTask();
	
	runFirstTask2(); // ����֮�󲻻᷵�أ��·���return 0��ʵûʲô����
	
	return 0;
}

