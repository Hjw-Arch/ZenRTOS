#ifndef RT_MUTEX_H
#define RT_MUTEX_H

#include "rt_event.h"

typedef struct _mutex {
	eventCtrlBlock_t event;
	
	uint32_t lockedCount; // 锁定次数
	
	uint32_t ownerOriginalPriority; // 互斥锁拥有者的原始优先级
	
	task_t* owner;	// 互斥量的拥有者
}mutex_t;




#endif
