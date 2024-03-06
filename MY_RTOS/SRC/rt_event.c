#include "rt_event.h"
#include "rtConfig.h"
#include "lock.h"

void eventInit(eventCtrlBlock_t* ecb, eventType_t type){
	ecb->type = eventTypeUnknown;
	listHeadInit(&ecb->waitlist);
}

// 将任务阻塞在事件控制块上
// 此函数要求被操作的任务必须处于就绪态或运行态
// 此函数是否要加临界区保护待决定
void eventWait(eventCtrlBlock_t* event, task_t* task, void* msg, uint32_t state, uint32_t timeout) {
	uint32_t st = enterCritical();
	
	task->state |= state;
	task->waitEvent = event;
	task->eventMsg = msg;
	task->eventWaitResult = NO_ERROR;
	
	taskSched2Unready(task);
	
	listNodeInsert2Tail(&event->waitlist, &task->linkNode);
	
	if (timeout) {
		taskSched2Delay(task, timeout);
	}
	
	leaveCritical(st);
}


task_t* eventWakeUp(eventCtrlBlock_t* event, void* msg, uint32_t result) {	
	
	task_t* task = NULL;
	
	uint32_t st = enterCritical();
	
#ifdef HIGH_RT_MODE
	
	uint32_t flag = RTOS_PRIORITY_COUNT;
	for (listNode* node = event->waitlist.firstNode; node != &event->waitlist.headNode; node = node->next) {
		task_t* temptask = getListNodeParent(node, task_t, linkNode);
		if (temptask->priority < flag) {
			flag = temptask->priority;
			task = temptask;
		}
	}
	if (task) {
		listRemove(&event->waitlist, &task->linkNode);
	}

#else
	
	listNode* node = NULL;
	if ((node = listRemoveFirst(&event->waitlist)) != NULL) {
		task = getListNodeParent(node, task_t, linkNode);
	}
	
#endif
	
	if (task == NULL) {
		return NULL;
	}
	
	task->waitEvent = NULL;
	task->eventMsg = msg;
	task->eventWaitResult = result;
	task->state &= ~TASK_STATUS_WAIT_MASK;
	
	if (task->state & TASK_STATUS_DELAY) {
		taskSched2Undelay(task);
	}
	
	taskSched2Ready(task);
	
	leaveCritical(st);
	
	return task;
}


// 这里不加锁也行

void eventRemoveTask(task_t* task, void* msg, uint32_t result) {
	uint32_t st = enterCritical();
	
	listRemove(&task->waitEvent->waitlist , &task->linkNode);
	
	task->waitEvent = NULL;
	task->eventMsg = msg;
	task->eventWaitResult = result;
	task->state &= ~TASK_STATUS_WAIT_MASK;
	
	leaveCritical(st);
}




