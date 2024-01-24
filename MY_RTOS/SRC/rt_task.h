#ifndef RT_TASK_H
#define RT_TASK_H

#include <stdint.h>
#include "rtConfig.h"

// 定义任务堆栈的类型为uint32
typedef uint32_t taskStack_t;

// 定义任务结构
typedef struct _t_Task {
	taskStack_t *stack;  // 任务的栈指针
	
	uint32_t delayTicks; // 任务延时计数器，在调用延时函数时每SysTick中断减一
	
	uint32_t priority;
}task_t;

extern task_t _idleTask;
extern task_t* currentTask;
extern task_t* nextTask;
extern task_t* idleTask;
extern task_t* taskTable[RTOS_PRIORITY_COUNT];
extern taskStack_t idleTaskEnv[512];


void runFirstTask(void);
void runFirstTask2(void);
void taskSwitch(void);
void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack, uint32_t priority);
void taskSched(void);
void taskDelay (uint32_t ms);
void setSysTick(uint32_t ms);
void idleTaskEntry (void* param);

#endif
