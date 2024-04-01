#include "semaphore.h"
#include "lock.h"


void semInit(sem_t* semaphore, uint32_t initCount, uint32_t maxCount) {

	eventInit(&semaphore->event, EVENT_TYPE_SEMAPHORE);
	
	semaphore->maxCount = maxCount;
	
	if (maxCount == 0) {
		semaphore->counter = initCount;
	}else {
		semaphore->counter = (maxCount > initCount) ? initCount : maxCount;
	}
	
}

uint32_t semWait(sem_t* semaphore, uint32_t waitTime) {
	uint32_t st = enterCritical();
	
	if (semaphore->counter > 0) {
		--semaphore->counter;
		leaveCritical(st);
		return NO_ERROR;
	}
	
	eventWait(&semaphore->event, currentTask, NULL, TASK_STATUS_WAIT_SEMAPHORE, waitTime);
	
	leaveCritical(st);
	
	taskSched();
	
	return currentTask->eventWaitResult;
}

uint32_t semGetWithNoWait(sem_t* semaphore) {
	uint32_t st = enterCritical();
	
	if (semaphore->counter > 0) {
		--semaphore->counter;
		leaveCritical(st);
		return NO_ERROR;
	}else {
		leaveCritical(st);
		return ERROR_RESOURCE_UNAVAILABLE;
	}
}

// semaphore->counter被一直加的话有可能溢出
void semPost(sem_t* semaphore) {
	uint32_t st = enterCritical();
	
	if (eventGetWaitNum(&semaphore->event) > 0) {
		task_t* task = eventWakeUp(&semaphore->event, NULL, NO_ERROR);
		if (task->priority < currentTask->priority) {
			taskSched();
		}
	}else {
		++semaphore->counter;
		if ((semaphore->maxCount != 0) && (semaphore->counter > semaphore->maxCount)) {
			semaphore->counter = semaphore->maxCount;
		}
	}
	
	leaveCritical(st);
}


uint32_t semDestory(sem_t* semaphore) {
	uint32_t st = enterCritical();
	
	uint32_t count = eventRemoveAllTask(&semaphore->event, NULL, ERROR_DELETED);
	
	semaphore->counter = 0;
	
	if (count > 0) {
		taskSched();
	}
	
	leaveCritical(st);
	
	return count;
}


semInfo_t semGetInfo(sem_t* semaphore) {
	semInfo_t info;
	
	uint32_t st = enterCritical();
	
	info.counter = semaphore->counter;
	info.maxcount = semaphore->maxCount;
	info.waitTaskNum = eventGetWaitNum(&semaphore->event);
	
	leaveCritical(st);
	
	return info;
}

// 一种更为高效的方式
/**
void semGetInfo(sem_t* semaphore, semInfo_t* info) {
	
	uint32_t st = enterCritical();
	
	info->counter = semaphore->counter;
	info->maxcount = semaphore->maxCount;
	info->waitTaskNum = eventGetWaitNum(&semaphore->event);
	
	leaveCritical(st);
	
}
**/















