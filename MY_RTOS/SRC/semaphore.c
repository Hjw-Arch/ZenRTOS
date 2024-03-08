#include "semaphore.h"
#include "lock.h"


void semInit(sem_t* semaphore, int initCount, uint32_t maxCount) {
	if (initCount < 0) {
		return;
	}
	
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
	
	if (--semaphore->counter >= 0) {
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
	
	if (++semaphore->counter <= 0) {
		task_t* task = eventWakeUp(&semaphore->event, NULL, NO_ERROR);
		if (task->priority < currentTask->priority) {
			taskSched();
		}
	}else {
		if ((semaphore->maxCount != 0) && (semaphore->counter > semaphore->maxCount)) {
			semaphore->counter = semaphore->maxCount;
		}
	}
	
	leaveCritical(st);
}



