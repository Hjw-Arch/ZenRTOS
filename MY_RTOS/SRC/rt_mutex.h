#ifndef RT_MUTEX_H
#define RT_MUTEX_H

#include "rt_event.h"

typedef struct _mutex {
	eventCtrlBlock_t event;
	
	uint32_t lockedCount; // 锁定次数
	
	uint32_t ownerOriginalPriority; // 互斥锁拥有者的原始优先级
	
	task_t* owner;	// 互斥量的拥有者
}mutex_t;

typedef struct _mutexinfo {
	uint32_t lockedCount;
	task_t* owner;
	uint32_t ownerOriginalPriority;
	uint32_t inheritedPriority;	//继承优先级，即拥有者当前实际的优先级
	uint32_t waitTaskNum;
}mutexInfo_t;

void mutexInit(mutex_t* mutex);
uint32_t mutexWait(mutex_t* mutex, uint32_t waitTime);
uint32_t mutexGetWithNoWait(mutex_t* mutex);
uint32_t mutexPost(mutex_t* mutex);
uint32_t mutexDestory(mutex_t* mutex);
mutexInfo_t mutexGetInfo(mutex_t* mutex);
// void mutexGetInfo(mutex_t* mutex, mutexInfo_t* info); //相较于上一个函数更高效的写法

#endif
