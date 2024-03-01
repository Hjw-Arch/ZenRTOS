#include "rt_task.h"
#include "lock.h"
#include "ARMCM3.h"

Bitmap taskPriorityBitmap;

task_t _idleTask;

task_t* currentTask;
task_t* nextTask;
task_t* idleTask;
listHead taskTable[RTOS_PRIORITY_COUNT];

taskStack_t idleTaskEnv[512];


// 任务初始化
void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack, uint32_t priority) {
	// 进入中断/异常时， 硬件会自动将8个寄存器压栈，顺序是xPSR、PC(R15)、LR(R14)、R12以及R3-R0
	*(--stack) = (unsigned long)(1 << 24); // xPSR中第24位，即T标志位设置为1，否则进入ARM模式，这在CM3上不允许！
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x03;
	*(--stack) = (unsigned long)0x02;
	*(--stack) = (unsigned long)0x01;
	*(--stack) = (unsigned long)param;
	
	// 手动保存R11-R4寄存器
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x09;
	*(--stack) = (unsigned long)0x08;
	*(--stack) = (unsigned long)0x07;
	*(--stack) = (unsigned long)0x06;
	*(--stack) = (unsigned long)0x05;
	*(--stack) = (unsigned long)0x04;
	
		
	task->stack = stack;
	task->delayTicks = 0;
	task->priority = priority;
	task->state = TASK_STATUS_READY;	// 初始状态为就绪态
	task->slice = TIME_SLICE;
	task->suspendCounter = 0;
	task->clean = NULL;
	task->cleanParam = NULL;
	task->requestDeleteFlag = 0;
	listNodeInit(&(task->delayNode)); // 初始化延时结点
	listNodeInit(&(task->linkNode)); // 初始化任务结点
	
	// 将task加入优先级队列，并将对应的位图置位
	taskSched2Ready(task);
}



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
		taskSwitch();
	}
	
	leaveCritical(st);
}


void idleTaskEntry (void* param) {
	while(1) ;
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

// 将任务加入就绪表，即将任务加入就绪态
void taskSched2Ready(task_t* task) {
	listNodeInsert2Tail(&taskTable[task->priority], &task->linkNode);
	bitmapSet(&taskPriorityBitmap, task->priority);
}

// 将任务从就绪表中移除，即将任务从就绪态移除
void taskSched2Unready(task_t* task) {
	listRemove(&taskTable[task->priority], &task->linkNode);
	if (getListNodeNum(&taskTable[task->priority]) == 0){
		bitmapClear(&taskPriorityBitmap, task->priority);
	}
}

// 将任务加入延时队列，状态设置为延时态
void taskSched2Delay(task_t* task) {
	listNodeInsert2Head(&taskDelayedList, &task->delayNode);
	task->state |= TASK_STATUS_DELAY;
}

// 将任务移除延时队列，将延时态清除
void taskSched2Undelay(task_t* task) {
	listRemove(&taskDelayedList, &task->delayNode);
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
void taskWakeUp(task_t* task) {
	uint32_t st = enterCritical();
	
	if (task->state & TASK_STATUS_SUSPEND) {
		if (--task->suspendCounter == 0) {
			task->state &= ~TASK_STATUS_SUSPEND;
			taskSched2Ready(task);
			taskSched();
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
	}
	
	if (!(task->state & TASK_STATUS_SUSPEND)) {
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
	uint32_t st = enterCritical();
	
	task->requestDeleteFlag = 1;
	
	leaveCritical(st);
}

uint8_t taskIsRequestedDelete(task_t* task) {
	uint8_t delete;
	
	uint32_t st = enterCritical();
	
	delete = task->requestDeleteFlag;
	
	leaveCritical(st);
	
	return delete;
}

void taskDeleteSelf(void) {
	uint32_t st = enterCritical();
	
	taskSched2Unready(currentTask);
	
	if (currentTask->clean) {
		currentTask->clean(currentTask->cleanParam);
	}
	
	taskSched();
	
	leaveCritical(st);
}
