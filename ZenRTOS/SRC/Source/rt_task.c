#include "rt_task.h"
#include "lock.h"
#include <string.h>
#include "rt_time.h"
#include "rt_hooks.h"
#if CORTEX_M4_FPU_OPENED == 1
#include "stm32f4xx.h"
#else
#include "ARMCM3.h"
#endif

Bitmap taskPriorityBitmap;

task_t* currentTask;
task_t* nextTask;

listHead taskTable[RTOS_PRIORITY_COUNT];

extern task_t* idleTask;


// 任务初始化
void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack, uint32_t priority, uint32_t stackSize) {
	task->stackBase = stack;
	task->stackSize = stackSize;
	memset(stack, 0, stackSize);
	
	taskStack_t* stackTop = stack + stackSize / sizeof(taskStack_t);
	
	// 进入中断/异常时， 硬件会自动将8个寄存器压栈，顺序是xPSR、PC(R15)、LR(R14)、R12以及R3-R0
#if CORTEX_M4_FPU_OPENED == 1
	stackTop -= 18;
#endif
	*(--stackTop) = (unsigned long)(1 << 24); // xPSR中第24位，即T标志位设置为1，否则进入ARM模式，这在CM3上不允许！
	*(--stackTop) = (unsigned long)entry;
	*(--stackTop) = (unsigned long)0x14;
	*(--stackTop) = (unsigned long)0x12;
	*(--stackTop) = (unsigned long)0x03;
	*(--stackTop) = (unsigned long)0x02;
	*(--stackTop) = (unsigned long)0x01;
	*(--stackTop) = (unsigned long)param;
	
	// 手动保存R11-R4寄存器
	*(--stackTop) = (unsigned long)0x11;
	*(--stackTop) = (unsigned long)0x10;
	*(--stackTop) = (unsigned long)0x09;
	*(--stackTop) = (unsigned long)0x08;
	*(--stackTop) = (unsigned long)0x07;
	*(--stackTop) = (unsigned long)0x06;
	*(--stackTop) = (unsigned long)0x05;
	*(--stackTop) = (unsigned long)0x04;
	
		
	task->stackTop = stackTop;
	task->delayTicks = 0;
	task->priority = priority;
	task->state = TASK_STATUS_READY;	// 初始状态为就绪态
	task->slice = TIME_SLICE;
	task->suspendCounter = 0;
	task->clean = NULL;
	task->cleanParam = NULL;
	task->requestDeleteFlag = 0;
	task->waitEvent = NULL;
	task->eventMsg = NULL;
	task->eventWaitResult = NO_ERROR;
	listNodeInit(&(task->delayNode)); // 初始化延时结点
	listNodeInit(&(task->linkNode)); // 初始化任务结点
	
	// 将task加入优先级队列，并将对应的位图置位
	taskSched2Ready(task);

#if FUNCTION_HOOKS_ENABLE == 1
	hooksTaskInit(task);
#endif

	if(currentTask != idleTask && currentTask->priority > task->priority) {
		taskSched();
	}
}


// 由于调用此函数的所有函数都加了锁，此函数不必加锁
// 任务调度函数，来决定下一个运行的任务是哪个
void taskSched(void) {
	uint32_t st = enterCritical();
	
	// 如果调度锁计数器大于0，则不切换任务
	if (schedLockCount > 0) {
		leaveCritical(st);
		return;
	}
	
	// 寻找最高优先级的任务
	task_t* tempTask = getHighestReadyTask();
	
	// 如果最高优先级的任务不是当前的任务，则切换到最高优先级的任务进行运行，否则不切换
	if (tempTask != currentTask) {
		nextTask = tempTask;
		
#if FUNCTION_HOOKS_ENABLE == 1
		hooksTaskSwitch(currentTask, nextTask);
#endif
		
		taskSwitch();
	}
	
	leaveCritical(st);
}

void taskYield(void) {
	uint32_t st = enterCritical();
	
	// 如果调度锁计数器大于0，则不切换任务
	if (schedLockCount > 0) {
		leaveCritical(st);
		return;
	}
	
	// 寻找最高优先级的任务
	uint32_t hightPriorityTask = bitmapGetFirstSet(&taskPriorityBitmap);

	
	if (hightPriorityTask == currentTask->priority) {
		listNode* node = listRemoveFirst(&taskTable[hightPriorityTask]);
		listNodeInsert2Tail(&taskTable[hightPriorityTask], node);
	}
	
	listNode* node = getFirstListNode(&taskTable[hightPriorityTask]);
	
	nextTask = getListNodeParent(node, task_t, linkNode);
	
	// 如果最高优先级的任务不是当前的任务，则切换到最高优先级的任务进行运行，否则不切换

#if FUNCTION_HOOKS_ENABLE == 1
		hooksTaskSwitch(currentTask, nextTask);
#endif
	if (nextTask != currentTask) {
		taskSwitch();
	}

	leaveCritical(st);
}


task_t* getHighestReadyTask(void) {
	uint32_t hightPriorityTask = bitmapGetFirstSet(&taskPriorityBitmap);
	listNode* node = getFirstListNode(&taskTable[hightPriorityTask]);
	return (task_t*)getListNodeParent(node, task_t, linkNode);
}

// 初始化任务延时队列
void taskDelayedListInit(void) {
	listHeadInit(&taskDelayedList);
}

// 应该做检查，检查任务的状态，如果处于延时、挂起、删除状态就不应该将其加入就绪表
// 将任务加入就绪表，即将任务加入就绪态
// 非原子操作
void taskSched2Ready(task_t* task) {
	listNodeInsert2Tail(&taskTable[task->priority], &task->linkNode);
	task->state = TASK_STATUS_READY;
	bitmapSet(&taskPriorityBitmap, task->priority);
}

// 将任务从就绪表中移除，即将任务从就绪态移除
void taskSched2Unready(task_t* task) {
	
	if (!checkNodeIsInList(&taskTable[task->priority], &task->linkNode)) {
		return;
	}
	
	listRemove(&taskTable[task->priority], &task->linkNode);
	
	task->state &= ~TASK_STATUS_READY;
	
	if (getListNodeNum(&taskTable[task->priority]) == 0){
		bitmapClear(&taskPriorityBitmap, task->priority);
	}
}

// 将任务加入延时队列，状态设置为延时态
void taskSched2Delay(task_t* task, uint32_t ms) {
	if (ms < SYS_TICK) ms = SYS_TICK;
	currentTask->delayTicks = (ms + SYS_TICK / 2) / SYS_TICK; // 四舍五入算法
	listNodeInsert2Head(&taskDelayedList, &task->delayNode);
	task->state |= TASK_STATUS_DELAY;
}

// 将任务移除延时队列，将延时态清除
void taskSched2Undelay(task_t* task) {
	
	if (!checkNodeIsInList(&taskDelayedList, &task->delayNode)) {
		return;
	}
	
	listRemove(&taskDelayedList, &task->delayNode);
	
	task->delayTicks = 0;
	
	task->state &= ~TASK_STATUS_DELAY;
}


// 可能被任务调用，要加锁
void taskSuspend(task_t* task) {
	uint32_t st = enterCritical();
	
	if (task->state & TASK_STATUS_DELAY) {
		leaveCritical(st);
		return;
	}
	
	if (task->suspendCounter++ == 0) {
		taskSched2Unready(task);
		task->state |= TASK_STATUS_SUSPEND;
		if (task == currentTask) {
			taskSched();
		}
	}
	
	leaveCritical(st);
}

// 唤醒挂起的任务
// 2024.3.8修改了调度算法，若唤醒的任务优先级大于当前任务才进行任务调度
void taskWakeUp(task_t* task) {
	uint32_t st = enterCritical();
	
	if (task->state & TASK_STATUS_SUSPEND) {
		if (--task->suspendCounter == 0) {
			task->state &= ~TASK_STATUS_SUSPEND;
			taskSched2Ready(task);
			if (task->priority < currentTask->priority) {
				taskSched();
			}
		}
	}
	
	leaveCritical(st);
}

// 设置任务的清理回调函数
void taskSetCleanCallFunc (task_t* task, void (*clean)(void* param), void* param) {
	task->clean = clean;
	task->cleanParam = param;
}

// 强制删除任务
void taskForceDelete (task_t* task) {
	uint32_t st = enterCritical();
	
	if (task->state & TASK_STATUS_DELAY) {
		taskSched2Undelay(task);
	}else if (!(task->state & TASK_STATUS_SUSPEND)) {
		taskSched2Unready(task);
	}
	
	if (task->clean) {
		task->clean(task->cleanParam);
	}
	
	if (task == currentTask) {
		taskSched();
	}
	
	leaveCritical(st);
}

//下面这两个函数的临界区保护不一定需要

// 请求删除任务
void taskRequestDelete(task_t* task){
	task->requestDeleteFlag = 1;
}

// 查询是否被请求删除
uint8_t taskIsRequestedDelete(task_t* task) {
	return task->requestDeleteFlag;
}

// 任务删除自己
void taskDeleteSelf(void) {
	uint32_t st = enterCritical();
	
	taskSched2Unready(currentTask);
	
	if (currentTask->clean) {
		currentTask->clean(currentTask->cleanParam);
	}
	
	taskSched();
	
	leaveCritical(st);
}

// 此处性能可以继续优化为下面的版本，避免了创建taskinfo和复制taskinfo的成本
// 但这种方式更符合编程习惯
taskInfo_t taskGetInfo(task_t* task) {
	taskInfo_t taskinfo;
	taskStack_t* stackEnd;
	uint32_t st = enterCritical();
	
	taskinfo.priority = task->priority;
	taskinfo.slice = task->slice;
	taskinfo.state = task->state;
	taskinfo.suspendCounter = task->suspendCounter;
	taskinfo.stackSize = task->stackSize;
	
	taskinfo.stackRtFreeSize = (task->stackTop - task->stackBase) * sizeof(taskStack_t);
	
	taskinfo.stackMinFreeSize = 0;
	stackEnd = task->stackBase;
	
	while((*stackEnd++ == 0) && (stackEnd <= task->stackTop)) {
		taskinfo.stackMinFreeSize++;
	}
	
	taskinfo.stackMinFreeSize *= sizeof(taskStack_t);
	
	leaveCritical(st);
	
	return taskinfo;
}

/** 此版本相较于上一个函数，开销更小一点
void taskGetInfo(task_t* task, taskInfo_t* taskinfo) {
	taskStack_t* stackEnd;
	uint32_t st = enterCritical();
	
	taskinfo->priority = task->priority;
	taskinfo->slice = task->slice;
	taskinfo->state = task->state;
	taskinfo->suspendCounter = task->suspendCounter;

	taskinfo->stackRtFreeSize = (task->stackTop - task->stackBase) * sizeof(taskStack_t);
	
	taskinfo->stackMinFreeSize = 0;
	stackEnd = task->stackBase;
	
	while((*stackEnd++ == 0) && (stackEnd <= task->stackTop)) {
		taskinfo->stackMinFreeSize++;
	}
	
	taskinfo->stackMinFreeSize *= sizeof(taskStack_t);
	
	leaveCritical(st);
}
**/
