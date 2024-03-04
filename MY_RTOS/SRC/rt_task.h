#ifndef RT_TASK_H
#define RT_TASK_H

#include <stdint.h>
#include "rtConfig.h"
#include "rtLib.h"

// 目前任务这里做的并不好，一个任务只能同时处于一个状态，这里太混乱了
#define TASK_STATUS_READY		0				// 任务状态：就绪态
#define TASK_STATUS_DELAY		(1 << 1)		// 任务状态：延时态
#define TASK_STATUS_SUSPEND		(1 << 2)		// 任务状态：挂起态
#define TASK_STATUS_DESTORYED	(1 << 3)		// 任务状态：删除态


// 定义任务堆栈的类型为uint32
typedef uint32_t taskStack_t;

// 定义任务结构
typedef struct _t_Task {
	taskStack_t *stack;  // 任务的栈指针
	
	uint32_t slice; // 时间片
	
	uint32_t delayTicks; // 任务延时计数器，在调用延时函数时每SysTick中断减一
	
	listNode linkNode;
	
	listNode delayNode; // 延时队列结点
	
	uint32_t state; // 任务此时的状态
	
	uint32_t priority; // 任务的优先级
	
	uint32_t suspendCounter; // 挂起计数器
	
	// 任务删除相关：任务清理函数
	void (*clean) (void* param);
	void* cleanParam;
	uint8_t requestDeleteFlag;		//请求删除标记
	
}task_t;

// 任务状态查询结构，用于保存查询的任务状态
typedef struct _taskInfo {
	uint32_t state; // 任务此时的状态
	uint32_t slice; // 时间片
	uint32_t priority; // 任务的优先级
	uint32_t suspendCounter; // 挂起计数器
}taskInfo_t;

extern task_t _idleTask;
extern task_t* currentTask;
extern task_t* nextTask;
extern task_t* idleTask;
extern listHead taskTable[RTOS_PRIORITY_COUNT];
extern taskStack_t idleTaskEnv[512];

extern listHead taskDelayedList;

extern Bitmap taskPriorityBitmap;


void runFirstTask(void);
void runFirstTask2(void);
void taskSwitch(void);

void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack, uint32_t priority);
void taskSched(void);

void idleTaskEntry (void* param);
task_t* getHighestReadyTask(void);
void taskDelayedListInit(void);
void taskSched2Ready(task_t* task);
void taskSched2Unready(task_t* task);

void taskSched2Delay(task_t* task);
void taskSched2Undelay(task_t* task);

void taskSuspend(task_t* task);
void taskWakeUp(task_t* task);

void taskSetCleanCallFunc (task_t* task, void (*clean)(void* param), void* param);
void taskForceDelete (task_t* task);
void taskRequestDelete(task_t* task);
uint8_t taskIsRequestedDelete(task_t* task);
void taskDeleteSelf(void);

taskInfo_t getTaskInfo(task_t* task);
// void getTaskInfo(task_t* task, taskInfo_t* taskinfo); // 此版本相较于上一个开销更小一点，不过小的不多

#endif
