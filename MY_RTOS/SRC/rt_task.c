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

listHead taskDelayedList;		// 延时队列


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
	listNodeInit(&(task->delayNode)); // 初始化延时结点
	listNodeInit(&(task->linkNode)); // 初始化任务结点
	
	// 将task加入优先级队列，并将对应的位图置位
	listNodeInsert2Head(&taskTable[priority], &(task->linkNode));
	
	bitmapSet(&taskPriorityBitmap, priority);
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


// 将任务加入就绪表
void taskSched2Ready(task_t* task) {
	listNodeInsert2Head(&taskTable[task->priority], &task->linkNode);
	bitmapSet(&taskPriorityBitmap, task->priority);
}

// 将任务从就绪表中移除
void taskSched2Unready(task_t* task) {
	listRemove(&taskTable[task->priority], &task->linkNode);
	if (getListNodeNum(&taskTable[task->priority]) == 0){
		bitmapClear(&taskPriorityBitmap, task->priority);
	}
}

// 要实现任务延时，需要使用定时器，而且每个任务都配备一个定时器才行，但是硬件只有一个定时器而任务数量很多
// 因此可以利用SysTick这个硬件定时器来实现软定时器
// 因为SysTick周期性触发中断，因此可以以这个周期为最基本的软件定时器的时间单位
// 每触发一次SysTick就将软定时器的值-1即可
// 因此软定时器的定时时间都是SysTick中断的倍数
// 中断处理也需要时间，不能太频繁这样会导致系统在切换任务这个事情上占用太多资源，本末倒置，一般设置为10ms-100ms就可以


// 软定时器延时精度并不准确，要注意使用场合 eg：延时一个单位，在两个SysTick中间开始延时，那么只能延时半个SysTick中断周期。
// 假如在将要触发定时器中断的时候发生了更高级别的中断，会导致延时时间变长

void taskDelay (uint32_t ms) {
	if (ms < SYS_TICK) ms = SYS_TICK;
	
	uint32_t st = enterCritical();
	
	currentTask->delayTicks = (ms + SYS_TICK / 2) / SYS_TICK; // 四舍五入算法
	
	taskWait(currentTask);
	
	taskSched2Unready(currentTask);
	
	leaveCritical(st);
	
	taskSched();
}

void taskTimeSliceHandler() {
	// 待完善：这里直接使用了list内部的元素进行迭代，没有进行封装，可以实现一个链表迭代器来封装一下
	for (listNode* node = taskDelayedList.firstNode; node != &(taskDelayedList.headNode); node = node->next) {
		task_t *task = getListNodeParent(node, task_t, delayNode);
		if (--task->delayTicks == 0) {
			taskWakeUp(task);
			taskSched2Ready(task);
		}
	}
	
	if (--currentTask->slice == 0) {
		if (getListNodeNum(&taskTable[currentTask->priority]) > 1) {
			listRemoveFirst(&taskTable[currentTask->priority]);
			listNodeInsert2Tail(&taskTable[currentTask->priority], &currentTask->linkNode);
		}
		currentTask->slice = TIME_SLICE;
	}
	
	taskSched();
}

// 设置SysTick定时中断的时间（中断周期，任务的时间片） 
void setSysTick(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; // 设定预载值
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);  //设定优先级
	SysTick->VAL = 0; //设定定时器值
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; //使用内核定时器；启动定时中断；使能SysTick
}

void SysTick_Handler() {
	taskTimeSliceHandler();
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

void taskWait(task_t* task) {
	listNodeInsert2Head(&taskDelayedList, &task->delayNode);
	task->state |= TASK_STATUS_DELAY;
}

void taskWakeUp(task_t* task) {
	listRemove(&taskDelayedList, &task->delayNode);
	task->state &= ~TASK_STATUS_DELAY;
}
