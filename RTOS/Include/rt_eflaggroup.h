#ifndef RT_EFLAGGROUP_H
#define RT_EFLAGGROUP_H

#include <stdint.h>
#include "rt_event.h"

#define EFLAGGROUP_RET			(0 << 0)
#define EFLAGGROUP_SET			(1 << 0)
#define EFLAGGROUP_ANY			(0 << 1)
#define EFLAGGROUP_ALL			(1 << 1)

#define EFLAGGROUP_ALL_SET		(EFLAGGROUP_SET | EFLAGGROUP_ALL)
#define EFLAGGROUP_ANY_SET		(EFLAGGROUP_SET | EFLAGGROUP_ANY)
#define EFLAGGROUP_ALL_RET		(EFLAGGROUP_RET | EFLAGGROUP_ALL)
#define EFLAGGROUP_ANY_RET		(EFLAGGROUP_RET | EFLAGGROUP_ANY)

#define EFLAGGROUP_CLEAR_AFTER	(1 << 7)

typedef struct _eflaggroup {
	uint32_t flags;		// 最多包含32个事件标志
	eventCtrlBlock_t event;
}eFlagGroup_t;

typedef struct _eflaggroupinfo {
	uint32_t flags;
	uint32_t waitTaskNum;
}eFlagGroupInfo_t;


void eFlagGroupInit(eFlagGroup_t* eflaggroup, uint32_t flags);
uint32_t eFlagGroupWait(eFlagGroup_t* eflaggroup, uint32_t waitFlagType, uint32_t expectedFlag, 
					uint32_t* resultFlag, uint32_t waitTime);
uint32_t eFlagGroupGetWithNoWait(eFlagGroup_t* eflaggroup, uint32_t waitFlagType, uint32_t expectedFlag, 
					uint32_t* resultFlag);

void eFlagGroupPost(eFlagGroup_t* eFlagGroup, uint32_t flags, uint32_t isSet);
uint32_t eFlagGroupDestory(eFlagGroup_t* eFlagGroup);
eFlagGroupInfo_t eFlagGroupGetInfo(eFlagGroup_t* eFlagGroup);
// void eFlagGroupGetInfo(eFlagGroup_t* eFlagGroup, eFlagGroupInfo_t* info); // 上一函数的高效率版本

#endif
