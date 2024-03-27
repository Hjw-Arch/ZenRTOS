#include "rt_timer.h"
#include "semaphore.h"
#include "lock.h"

// 同一个软定时器最好只在同一个任务内调用！

static listHead timerStrictList;	// 在时间片中断内处理的定时器列表
static listHead timerLooseList;	// 在定时器任务中处理的定时器列表
static sem_t semForTimerProtect;
static sem_t semForTimerNotify;	// 用于通知定时器任务开始执行,用事件标志组也行

static task_t timerTask;
static taskStack_t timerTaskStack[TIMER_TASK_STACK_SIZE];

// 工具函数，用于遍历定时器列表执行倒计时操作
static void timerCallFuncList(listHead* timerList) {
	for (listNode* node = timerList->firstNode; node != &timerList->headNode; node = node->next) {
		timer_t* timer = getListNodeParent(node, timer_t, linkNode);
		if (timer->rtDelayTicks > 0) {
			if (--timer->rtDelayTicks == 0) {
				timer->state = TIMER_STATE_RUNNING;
				timer->timerFunc(timer->arg);
				timer->state = TIMER_STATE_STARTED;
				
				if (timer->durationTicks != 0) {
					timer->rtDelayTicks = timer->durationTicks;
				} else {
					listRemove(timerList, &timer->linkNode);
					timer->state = TIMER_STATE_STOPPED;
				}
			}
		}
	}
}

// 定时器任务函数
static void timerTaskFunc (void* param) {
	while (1) {
		semWait(&semForTimerNotify, 0);
		// 加信号量处理是因为timerCallFuncList涉及到对timerLooseList的访问
		semWait(&semForTimerProtect, 0);
		
		timerCallFuncList(&timerLooseList);
		
		semPost(&semForTimerProtect);
	}
	
}

// 供给时间片中断调用，每一个tick发送一个信号，唤醒timerTaskFunc开始执行
// 同时会处理精准软定时器列表
// 由于是供给中断调用，不需要加临界区保护了
void timerFuncPost(void) {
	timerCallFuncList(&timerStrictList);
	
	semPost(&semForTimerNotify);
}

// 启动软定时器功能
void timerFuncInit(void) {
	listHeadInit(&timerStrictList);
	listHeadInit(&timerLooseList);
	semInit(&semForTimerProtect, 1, 1);
	semInit(&semForTimerNotify, 0, 0);
	
#if TIMER_TASK_PRIORITY >= (RTOS_PRIORITY_COUNT - 1)
	#error "The priority of timerTask must be greater then (RTOS_PRIORITY_COUNT - 1)"
#endif	
	
	taskInit(&timerTask, timerTaskFunc, NULL, &timerTaskStack[TIMER_TASK_STACK_SIZE], TIMER_TASK_PRIORITY);
}


// 定时器要在任务内使用，最好不要定义成全局变量
// 初始化一个定时器
void timerInit(timer_t* timer, uint32_t originalDelayTicks, uint32_t durationTicks, void (*timerFunc)(void* arg), void* arg, uint32_t config) {
	listNodeInit(&timer->linkNode);
	timer->originalDelayTicks = originalDelayTicks;
	timer->durationTicks = durationTicks;
	timer->timerFunc = timerFunc;
	timer->arg = arg;
	timer->config = config;
	
	if (originalDelayTicks == 0) {
		timer->rtDelayTicks = durationTicks;
	} else {
		timer->rtDelayTicks = timer->originalDelayTicks;
	}
	
	timer->state = TIMER_STATE_CREATED;
}

// 定时器最好只在一个任务内调用
// 这里对全局变量的保护没有做好
// 让定时器开始计时
void timerStart(timer_t* timer) {
	if (timer->state == TIMER_STATE_CREATED || timer->state == TIMER_STATE_STOPPED) {
		if (timer->config & TIMER_CONFIG_TYPE_STRICT) {
			// 中断处理函数也会访问timerStrictList，因此使用临界区
			uint32_t st = enterCritical();
			
			listNodeInsert2Tail(&timerStrictList, &timer->linkNode);
			timer->state = TIMER_STATE_STARTED;
			timer->rtDelayTicks = timer->originalDelayTicks ? timer->originalDelayTicks : timer->durationTicks;
			
			leaveCritical(st);
		} else {
			// timerLooseList只会被timerTask访问，用信号量就可以
			semWait(&semForTimerProtect, 0);
			listNodeInsert2Tail(&timerLooseList, &timer->linkNode);
			timer->state = TIMER_STATE_STARTED;
			timer->rtDelayTicks = timer->originalDelayTicks ? timer->originalDelayTicks : timer->durationTicks;
			semPost(&semForTimerProtect);
		}
	}
	
}

// 让定时器暂停
// 在running的时候其实暂停不了，因为要么在定时器中断中running，要么running的时候加锁了
void timerStop(timer_t* timer) {
	if (timer->state == TIMER_STATE_RUNNING || timer->state == TIMER_STATE_STARTED) {
		if (timer->config & TIMER_CONFIG_TYPE_STRICT) {
			uint32_t st = enterCritical();
			listRemove(&timerStrictList, &timer->linkNode);
			timer->state = TIMER_STATE_STOPPED;
			leaveCritical(st);
		} else {
			semWait(&semForTimerProtect, 0);
			listRemove(&timerLooseList, &timer->linkNode);
			timer->state = TIMER_STATE_STOPPED;
			semPost(&semForTimerProtect);
		}
	}
	
}

// 恢复定时器运行
void timerResume(timer_t* timer) {
	if (timer->state == TIMER_STATE_STOPPED) {
		
		if (timer->config & TIMER_CONFIG_TYPE_STRICT) {
			// 中断处理函数也会访问timerStrictList，因此使用临界区
			uint32_t st = enterCritical();
			listNodeInsert2Tail(&timerStrictList, &timer->linkNode);
			timer->state = TIMER_STATE_STARTED;
			leaveCritical(st);
		} else {
			// timerLooseList只会被timerTask访问，用信号量就可以
			semWait(&semForTimerProtect, 0);
			listNodeInsert2Tail(&timerLooseList, &timer->linkNode);
			timer->state = TIMER_STATE_STARTED;
			semPost(&semForTimerProtect);
		}
	}
}

// 这个函数的保护没有做好
/**
void timerDestory(timer_t* timer) {
	timerStop(timer);
	timer->state = TIMER_STATE_DESTORYED;
}
**/

void timerDestory(timer_t* timer) {
	if (timer->state == TIMER_STATE_RUNNING || timer->state == TIMER_STATE_STARTED) {
		if (timer->config & TIMER_CONFIG_TYPE_STRICT) {
			uint32_t st = enterCritical();
			listRemove(&timerStrictList, &timer->linkNode);
			timer->state = TIMER_STATE_DESTORYED;
			leaveCritical(st);
		} else {
			semWait(&semForTimerProtect, 0);
			listRemove(&timerLooseList, &timer->linkNode);
			timer->state = TIMER_STATE_DESTORYED;
			semPost(&semForTimerProtect);
		}
	}
}

timerInfo_t timerGetInfo(timer_t* timer) {
	timerInfo_t info;
	uint32_t st = enterCritical();
	
	info.originalDelayTicks = timer->originalDelayTicks;
	info.durationDelayTicks = timer->durationTicks;
	info.timerFunc = timer->timerFunc;
	info.state = timer->state;
	info.arg = timer->arg;
	
	leaveCritical(st);
	return info;
}

// 更高效率的版本
/**
void timerGetInfo(timer_t* timer, timerInfo_t* info) {
	uint32_t st = enterCritical();
	
	info->originalDelayTicks = timer->originalDelayTicks;
	info->durationDelayTicks = timer->durationTicks;
	info->timerFunc = timer->timerFunc;
	info->state = timer->state;
	info->arg = timer->arg;
	
	leaveCritical(st);
}
**/
