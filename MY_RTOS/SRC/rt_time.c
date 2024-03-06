#include "rt_task.h"
#include "rtLib.h"
#include "lock.h"
#include "rt_event.h"

listHead taskDelayedList;		// 延时队列

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

	// 这里或许可以优化，因为taskSched2Delay应该包含taskSched2Unready
	taskSched2Delay(currentTask, ms);
	
	taskSched2Unready(currentTask);
	
	taskSched();
	
	leaveCritical(st);
	
}

void taskTimeSliceHandler() {
	// 待完善：这里直接使用了list内部的元素进行迭代，没有进行封装，可以实现一个链表迭代器来封装一下
	for (listNode* node = taskDelayedList.firstNode; node != &(taskDelayedList.headNode); node = node->next) {
		task_t *task = getListNodeParent(node, task_t, delayNode);
		if (--task->delayTicks == 0) {
			if(task->waitEvent) {
				eventRemoveTask(task, NULL, ERROR_TIMEOUT);
			}
			taskSched2Undelay(task);
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

void SysTick_Handler() {
	taskTimeSliceHandler();
}

