#include "RTOS.h"

void rtosInit() {
	// idletask
	schedLockInit();   //没必要
	bitmapInit(&taskPriorityBitmap);   //没必要
	taskDelayedListInit();
	// 初始化任务链表
	for (int i = 0; i < RTOS_PRIORITY_COUNT; ++i) {
		listHeadInit(&taskTable[i]);
	}

#if FUNCTION_CPUUSAGE_ENABLE == 0
	appInit();

#if FUNCTION_SOFTTIMER_ENABLE == 1
	timerFuncInit();
#endif
#endif
	
	// 初始化空闲任务
	taskInit(&_idleTask, idleTaskEntry, (void*)0, idleTaskEnv, RTOS_PRIORITY_COUNT - 1, sizeof(idleTaskEnv));
	idleTask = &_idleTask;
	
}

int main(){
	rtosInit();
	
	nextTask = getHighestReadyTask();

#if FUNCTION_CPUUSAGE_ENABLE == 0
	setSysTick(SYS_TICK);
#endif
	
	runFirstTask2(); // 运行之后不会返回，下方的return 0其实没什么作用
	
	return 0;
}

