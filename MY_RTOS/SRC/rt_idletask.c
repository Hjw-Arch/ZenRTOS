#include "rt_task.h"
#include "lock.h"
#include "rt_time.h"

#define TICKS_PER_SEC		(1000 / SYS_TICK)

void appInit(void);

// idletask
task_t* idleTask;
uint32_t idleCount;
uint32_t tickCount;
uint32_t maxIdleCount;

static float cpuUsage;

static uint32_t enableCpuUsageState;

static void cpuUsageStateInit(void) {
	enableCpuUsageState = 0;
	maxIdleCount = 0;
	idleCount = 0;
	cpuUsage = 0.0f;
}

void checkCpuUsage(void) {
	if (enableCpuUsageState == 0) {
		enableCpuUsageState = 1;
		tickCount = 0;
		return;
	}
	
	if (tickCount == TICKS_PER_SEC) {
		maxIdleCount = idleCount;
		idleCount = 0;
		
		unlockSched();
	} else if (tickCount % TICKS_PER_SEC == 0) {
		cpuUsage = 100 - (idleCount * 100.0 / maxIdleCount);
		idleCount = 0;
	}
	
}

static void cpuUsageSyncWithSysTick (void) {
	while (enableCpuUsageState == 0);
}

void idleTaskEntry (void* param) {
//	schedLockInit();   //没必要
	lockSched();
	appInit();

// 初始化时钟	
	setSysTick(SYS_TICK);
	
	cpuUsageSyncWithSysTick();
	
	while(1) {
		uint32_t st = enterCritical();
		idleCount++;
		leaveCritical(st);
	}
}

