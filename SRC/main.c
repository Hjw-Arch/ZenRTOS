#include "RTOS.h"


sem_t semForPrintf;

void rtosInit() {
	// idletask
	schedLockInit();   //û��Ҫ
	bitmapInit(&taskPriorityBitmap);   //û��Ҫ
	taskDelayedListInit();
	// ��ʼ����������
	for (int i = 0; i < RTOS_PRIORITY_COUNT; ++i) {
		listHeadInit(&taskTable[i]);
	}

#if FUNCTION_CPUUSAGE_ENABLE == 0
	appInit();

#if FUNCTION_SOFTTIMER_ENABLE == 1
	timerFuncInit();
#endif
#endif
	
	// ��ʼ����������
	taskInit(&_idleTask, idleTaskEntry, (void*)0, idleTaskEnv, RTOS_PRIORITY_COUNT - 1, sizeof(idleTaskEnv), "IDLETASK");
	idleTask = &_idleTask;
	
	semInit(&semForPrintf, 1, 1);
}

int main(){
	rtosInit();
	
	nextTask = getHighestReadyTask();

#if FUNCTION_CPUUSAGE_ENABLE == 0
	setSysTick(SYS_TICK);
#endif
	
	runFirstTask2(); // ����֮�󲻻᷵�أ��·���return 0��ʵûʲô����
	
	return 0;
}

