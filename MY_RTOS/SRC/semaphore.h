#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "rt_event.h"

typedef struct _semaphore {
	eventCtrlBlock_t event;
	int counter;
	uint32_t maxCount;
}sem_t;

void semInit(sem_t* semaphore, int initCount, uint32_t maxCount);
uint32_t semWait(sem_t* semaphore, uint32_t waitTime);
uint32_t semGetWithNoWait(sem_t* semaphore);
void semPost(sem_t* semaphore);




#endif

