#include "rt_mutex.h"
#include "lock.h"


void mutexInit(mutex_t* mutex) {
	eventInit(&mutex->event, EVENT_TYPE_MUTEX);
	mutex->lockedCount = 0;
	mutex->owner = NULL;
	mutex->ownerOriginalPriority = RTOS_PRIORITY_COUNT;
}

uint32_t mutexWait(mutex_t* mutex, uint32_t waitTime) {
	uint32_t st = enterCritical();
	
	if (mutex->lockedCount == 0) {
		mutex->owner = currentTask;
		mutex->ownerOriginalPriority = currentTask->priority;
		mutex->lockedCount++;
		
		leaveCritical(st);
		return NO_ERROR;
	}
	
	if (mutex->owner == currentTask) {
		mutex->lockedCount++;
		leaveCritical(st);
		return NO_ERROR;
	}
	
	if (currentTask->priority >= mutex->ownerOriginalPriority) {
		eventWait(&mutex->event, currentTask, NULL, TASK_STATUS_WAIT_MUTEX, waitTime);
		taskSched();
		leaveCritical(st);
		
		return currentTask->eventWaitResult;
	}
	
	if (currentTask->priority < mutex->ownerOriginalPriority) {
		if (mutex->owner->state & TASK_STATUS_READY) {
			taskSched2Unready(mutex->owner);
			mutex->owner->priority = currentTask->priority;
			taskSched2Ready(mutex->owner);
		}
		
		mutex->owner->priority = currentTask->priority;
		
		eventWait(&mutex->event, currentTask, NULL, TASK_STATUS_WAIT_MUTEX, waitTime);
		taskSched();
		leaveCritical(st);

		return currentTask->eventWaitResult;
	}
	
	leaveCritical(st);
	return NO_ERROR;
}


uint32_t mutexGetWithNoWait(mutex_t* mutex) {
	uint32_t st = enterCritical();
	
	if (mutex->lockedCount == 0) {
		mutex->owner = currentTask;
		mutex->ownerOriginalPriority = currentTask->priority;
		mutex->lockedCount++;
		
		leaveCritical(st);
		return NO_ERROR;
	}
	
	if (mutex->owner == currentTask) {
		mutex->lockedCount++;
		leaveCritical(st);
		return NO_ERROR;
	}
	
	leaveCritical(st);
	return ERROR_RESOURCE_UNAVAILABLE;
}


uint32_t mutexPost(mutex_t* mutex) {
	uint32_t st = enterCritical();

	if (currentTask != mutex->owner) {
		leaveCritical(st);
		return ERROR_UNMATCHED_OWNER;
	}
	
	if (mutex->lockedCount > 0) {
		if (--mutex->lockedCount == 0) {
			if (mutex->ownerOriginalPriority != currentTask->priority) {
				taskSched2Unready(currentTask);
				currentTask->priority = mutex->ownerOriginalPriority;
				taskSched2Ready(currentTask);
			}
			
			task_t* task = NULL;
			
			if ((task = eventWakeUp(&mutex->event, NULL, NO_ERROR)) != NULL) {
				mutex->owner = task;
				mutex->ownerOriginalPriority = task->priority;
				mutex->lockedCount++;
				
				if (task->priority < currentTask->priority) {
					taskSched();
				}
				
				leaveCritical(st);
				return NO_ERROR;
			}
		}
	}
	
	leaveCritical(st);
	return NO_ERROR;
}


uint32_t mutexDestory(mutex_t* mutex) {
	uint32_t count = 0;
	
	uint32_t st = enterCritical();
	
	// 这段函数没有意义，如果mutex->ownerOriginalPriority != mutex->owner->priority
	// 那么mutex->lockedCount一定不等于0
	// 因为post操作会将priority归位并将lockedCount++
	if (mutex->lockedCount == 0) {
		leaveCritical(st);
		return count;
	}
	
	if (mutex->ownerOriginalPriority != mutex->owner->priority) {
		if (mutex->owner->state & TASK_STATUS_READY) {
			taskSched2Unready(mutex->owner);
			mutex->owner->priority = mutex->ownerOriginalPriority;
			taskSched2Ready(mutex->owner);
		}
		else {
			mutex->owner->priority = mutex->ownerOriginalPriority;
		}
	}
	
	if ((count = eventRemoveAllTask(&mutex->event, NULL, ERROR_DELETED)) > 0) {
		taskSched();
	}
	
	leaveCritical(st);
	
	return count;
}

mutexInfo_t mutexGetInfo(mutex_t* mutex) {
	mutexInfo_t info;
	
	uint32_t st = enterCritical();
	
	info.owner = mutex->owner;
	if (info.owner == NULL) {
		info.inheritedPriority = RTOS_PRIORITY_COUNT;
		info.ownerOriginalPriority = RTOS_PRIORITY_COUNT;
	}else {
		info.inheritedPriority = mutex->owner->priority;
		info.ownerOriginalPriority = mutex->ownerOriginalPriority;
	}
	info.lockedCount = mutex->lockedCount;
	info.waitTaskNum = eventGetWaitNum(&mutex->event);
	
	leaveCritical(st);
	
	return info;
}

// 相较于上个函数更高效率的函数
/**
void mutexGetInfo(mutex_t* mutex, mutexInfo_t* info) {
	uint32_t st = enterCritical();
	
	info->owner = mutex->owner;	
	info->lockedCount = mutex->lockedCount;
	info->waitTaskNum = eventGetWaitNum(&mutex->event);
	info->inheritedPriority = mutex->owner->priority;
	info->ownerOriginalPriority = mutex->ownerOriginalPriority;
	
	leaveCritical(st);
}
**/

