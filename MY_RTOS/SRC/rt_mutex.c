#include "rt_mutex.h"


void mutexInit(mutex_t* mutex) {
	eventInit(&mutex->event, EVENT_TYPE_MUTEX);
	mutex->lockedCount = 0;
	mutex->owner = NULL;
	mutex->ownerOriginalPriority = RTOS_PRIORITY_COUNT;
}