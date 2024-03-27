#ifndef RT_TIMER_H
#define RT_TIMER_H

#include "rt_event.h"

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

void timerInit(timer_t* timer, uint32_t originalDelayTicks, uint32_t durationTicks, void (*timerFunc)(void* arg), void* arg, uint32_t config);
void timerFuncInit(void);
void timerFuncPost(void);
void timerStart(timer_t* timer);
void timerStop(timer_t* timer);
void timerResume(timer_t* timer);



#endif
