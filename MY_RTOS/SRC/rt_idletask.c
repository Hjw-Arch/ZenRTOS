#include "rt_task.h"
#include "lock.h"
#include "rt_time.h"
#include "rt_timer.h"

// һ���Ӳ�����tick��������SYS_TICK������ܱ�1000����������
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

// �к�û��û����
static void cpuUsageStateInit(void) {
	enableCpuUsageState = 0;
	maxIdleCount = 0;
	idleCount = 0;
	cpuUsage = 0.0f;
}

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
	
	
// ��ʼ��ʱ��	
	setSysTick(SYS_TICK);
	
	cpuUsageSyncWithSysTick();
	
	// ��������,���ܲ���Ҫ
	while(1) {
		uint32_t st = enterCritical();
		idleCount++;
		leaveCritical(st);
	}
}

// ����û��Ҫ����
// �����ʹ�ô˺���ʱ����tick�жϣ���ô�п��ܵ���ȡ����usage�������µ�
// �����������������ʹ�ô˺���ʱû�з���tick�жϵĻ���ȡ�ľ�������ֵ
// ���������tick�жϣ�����ֻ���ӳ��жϵķ�������ʵ��ȡ����Ҳ�������������ϵ�����ֵ
float cpuGetUsage(void) {
	return cpuUsage;
}

