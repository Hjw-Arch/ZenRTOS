#include "rt_task.h"
#include "lock.h"
#include "rt_time.h"
#include "rt_timer.h"

#include "rt_hooks.h"

// һ���Ӳ�����tick��������SYS_TICK������ܱ�1000����������
#define TICKS_PER_SEC		(1000 / SYS_TICK)

void appInit(void);

task_t _idleTask;
taskStack_t idleTaskEnv[TASK_STACK_SIZE];
task_t* idleTask;

#if FUNCTION_CPUUSAGE_ENABLE == 1
// idletask
uint32_t idleCount = 0;
uint32_t tickCount = 0;
uint32_t maxIdleCount = 0;

static float cpuUsage = 0.0f;

static uint32_t enableCpuUsageState = 0;

/**
// �к�û��û����
static void cpuUsageStateInit(void) {
	enableCpuUsageState = 0;
	maxIdleCount = 0;
	idleCount = 0;
	cpuUsage = 0.0f;
}
**/

// ��tick�ж�ʹ��
void checkCpuUsage(void) {
	// Ϊ�˵�һ�β���tick�жϵ�ʱ��ʼ���м���
	// ��ǰ������������idletask����æ�ȴ�״̬
	if (enableCpuUsageState == 0) {
		enableCpuUsageState = 1;
		tickCount = 0;
		return;
	}
	
	tickCount++;
	
	// �������һ���ӣ������һ���ڵ�������ֵ���رյ�����
	// Ȼ��ÿ��һ���Ӽ���һ��CPU������
	if (tickCount == TICKS_PER_SEC) {
		maxIdleCount = idleCount;
		idleCount = 0;
		
		
#if FUNCTION_SOFTTIMER_ENABLE == 1
		timerResetSemForTimerNotify();
#endif
		
		unlockSched();
	} else if (tickCount % TICKS_PER_SEC == 0) {
		cpuUsage = 100 - (idleCount * 100.0 / maxIdleCount);
		idleCount = 0;
	}
	
}

static void cpuUsageSyncWithSysTick (void) {
	while (enableCpuUsageState == 0);
}

#endif

void idleTaskEntry (void* param) {
	lockSched();
	
#if FUNCTION_SOFTTIMER_ENABLE == 1
	timerFuncInit();
#endif
	
// ��ʼ��ʱ��
	appInit();
	setSysTick(SYS_TICK);
#if FUNCTION_CPUUSAGE_ENABLE == 1
// æ�ȴ���һ��tick
	cpuUsageSyncWithSysTick();
#else
	unlockSched();
#endif
	// ��������,���ܲ���Ҫ
	while(1) {
#if FUNCTION_CPUUSAGE_ENABLE == 1
		uint32_t st = enterCritical();
		idleCount++;
		leaveCritical(st);
#endif

#if FUNCTION_HOOKS_ENABLE == 1
		hooksCpuIdle();
#endif
	}
	
	
}

// ����û��Ҫ����
// �����ʹ�ô˺���ʱ����tick�жϣ���ô�п��ܵ���ȡ����usage�������µ�
// �����������������ʹ�ô˺���ʱû�з���tick�жϵĻ���ȡ�ľ�������ֵ
// ���������tick�жϣ�����ֻ���ӳ��жϵķ�������ʵ��ȡ����Ҳ�������������ϵ�����ֵ

#if FUNCTION_CPUUSAGE_ENABLE == 1
float cpuGetUsage(void) {
	return cpuUsage;
}
#endif

