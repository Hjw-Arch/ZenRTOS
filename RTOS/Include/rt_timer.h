#ifndef RT_TIMER_H
#define RT_TIMER_H

#include "rt_event.h"

#define TIMER_TASK_STACK_SIZE		1024	// 定时器任务的堆栈大小
#define TIMER_TASK_PRIORITY			1		// 定时器任务的优先级，注意，不能和空闲任务的优先级一致，否则它两会按时间片交替运行

#define TIMER_CONFIG_TYPE_STRICT	(1 << 0)
#define TIMER_CONFIG_TYPE_LOOSE		(0 << 0)

typedef enum _timerState {
	TIMER_STATE_CREATED,
	TIMER_STATE_STARTED,
	TIMER_STATE_RUNNING,
	TIMER_STATE_STOPPED,
	TIMER_STATE_DESTORYED,
}timerState_t;

typedef struct _rt_timer {
	listNode linkNode;
	uint32_t originalDelayTicks;
	uint32_t durationTicks;
	uint32_t rtDelayTicks;
	void (*timerFunc) (void* arg);
	void* arg;
	uint32_t config;
	timerState_t state;
}timer_t;

typedef struct _timerinfo {
	uint32_t originalDelayTicks;
	uint32_t durationDelayTicks;
	void (*timerFunc)(void* arg);
	void* arg;
	uint32_t config;
	timerState_t state;
}timerInfo_t;

void timerInit(timer_t* timer, uint32_t originalDelayTicks, uint32_t durationTicks, void (*timerFunc)(void* arg), void* arg, uint32_t config);
void timerFuncInit(void);
void timerFuncPost(void);
void timerStart(timer_t* timer);
void timerStop(timer_t* timer);
void timerResume(timer_t* timer);
void timerDestory(timer_t* timer);
timerInfo_t timerGetInfo(timer_t* timer);
/**
// 更高效率的版本
void timerGetInfo(timer_t* timer, timerInfo_t* info);
**/


void timerResetSemForTimerNotify(void);		// 专用函数！禁止调用！


#endif
