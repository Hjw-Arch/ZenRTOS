#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "rt_event.h"


// 信号量的定义
typedef struct _semaphore {
	eventCtrlBlock_t event;
	uint32_t counter;
	uint32_t maxCount;
}sem_t;


// 信号量的状态结构，用于保存信号量的状态
typedef struct _seminfo {
	uint32_t counter;
	uint32_t maxcount;
	uint32_t waitTaskNum;
}semInfo_t;


void semInit(sem_t* semaphore, uint32_t initCount, uint32_t maxCount);
uint32_t semWait(sem_t* semaphore, uint32_t waitTime);
uint32_t semGetWithNoWait(sem_t* semaphore);
void semPost(sem_t* semaphore);
uint32_t semDestory(sem_t* semaphore);
semInfo_t semGetInfo(sem_t* semaphore);
// void semGetInfo(sem_t* semaphore, semInfo_t* info);	//相较于上一个函数的一种更为高效的实现方式



#endif
