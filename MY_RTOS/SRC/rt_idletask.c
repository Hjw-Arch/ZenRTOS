#include "rt_task.h"
#include "lock.h"
#include "rt_time.h"
#include "rt_timer.h"

// 一秒钟产生的tick数，所以SYS_TICK最好是能被1000除尽的整数
#define TICKS_PER_SEC		(1000 / SYS_TICK)

void appInit(void);

task_t _idleTask;
taskStack_t idleTaskEnv[TASK_STACK_SIZE];

// idletask
task_t* idleTask;
uint32_t idleCount = 0;
uint32_t tickCount = 0;
uint32_t maxIdleCount = 0;

static float cpuUsage = 0.0f;

static uint32_t enableCpuUsageState = 0;

// 有和没有没区别
static void cpuUsageStateInit(void) {
	enableCpuUsageState = 0;
	maxIdleCount = 0;
	idleCount = 0;
	cpuUsage = 0.0f;
}

// 给tick中断使用
void checkCpuUsage(void) {
	// 为了第一次产生tick中断的时候开始运行计数
	// 此前计数不开启，idletask处于忙等待状态
	if (enableCpuUsageState == 0) {
		enableCpuUsageState = 1;
		tickCount = 0;
		return;
	}
	
	tickCount++;
	
	// 如果到了一秒钟，计算出一秒内的最大计数值并关闭调度锁
	// 然后每隔一秒钟计算一次CPU利用率
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

	lockSched();
	appInit();
	
	
#ifdef SOFT_TIMER
	timerFuncInit();
#endif
	
	
// 初始化时钟	
	setSysTick(SYS_TICK);
	
	cpuUsageSyncWithSysTick();
	
	// 加锁保护,可能不需要
	while(1) {
		uint32_t st = enterCritical();
		idleCount++;
		leaveCritical(st);
	}
}

// 这里没必要加锁
// 如果在使用此函数时发生tick中断，那么有可能导致取到的usage不是最新的
// 但是如果加了锁，在使用此函数时没有发生tick中断的话获取的就是最新值
// 如果发生了tick中断，加锁只是延迟中断的发生，其实获取到的也不是真正意义上的最新值
float cpuGetUsage(void) {
	return cpuUsage;
}

