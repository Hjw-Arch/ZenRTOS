#include "rt_eflaggroup.h"
#include "lock.h"

#if FUNCTION_EFLAGGROUP_EBABLE == 1

void eFlagGroupInit(eFlagGroup_t* eflaggroup, uint32_t flags) {
	eventInit(&eflaggroup->event, EVENT_TYPE_EFLAGGROUP);
	eflaggroup->flags = flags;
}

// ���ı��flags��ʾ������Щ��־λ�Ǳ������
static uint32_t eFlagGroupCheckExpected(eFlagGroup_t* eFlagGroup, uint32_t waitEFlagType, uint32_t* flags) {
	uint32_t srcFlag = *flags;
	uint32_t isWaitSet = waitEFlagType & EFLAGGROUP_SET;
	uint32_t isWaitAll = waitEFlagType & EFLAGGROUP_ALL;
	uint32_t isClear = waitEFlagType & EFLAGGROUP_CLEAR_AFTER;
	
	uint32_t calFlag = isWaitSet ? (eFlagGroup->flags & srcFlag) : (~eFlagGroup->flags & srcFlag);
	
	if (((isWaitAll != 0) && (calFlag == srcFlag)) || ((isWaitAll == 0) && (calFlag != 0))) {
		if (isClear) {
			if (isWaitSet) {
				eFlagGroup->flags &= ~calFlag;
			} else {
				eFlagGroup->flags |= calFlag;
			}
		}
		*flags = calFlag;
		return 0;
	}
	
	*flags = calFlag;
	
	return 1;
}


uint32_t eFlagGroupWait(eFlagGroup_t* eflaggroup, uint32_t waitFlagType, uint32_t expectedFlag, 
					uint32_t* resultFlag, uint32_t waitTime) 
{	
	uint32_t result;
	uint32_t flags = expectedFlag;
	
	uint32_t st = enterCritical();
	
	result = eFlagGroupCheckExpected(eflaggroup, waitFlagType, &flags);
	
	// ����������
	if(result == 1) {
		currentTask->waitEventFlagType = waitFlagType;
		currentTask->waitEventFlags = expectedFlag;
		
		eventWait(&eflaggroup->event, currentTask, NULL, TASK_STATUS_WAIT_EFLAGGROUP, waitTime);
		taskSched();
		leaveCritical(st);
		
		*resultFlag = currentTask->waitEventFlags;
		return currentTask->eventWaitResult;
	}
	
	*resultFlag = flags;
	
	leaveCritical(st);
	
	return NO_ERROR;
}

uint32_t eFlagGroupGetWithNoWait(eFlagGroup_t* eflaggroup, uint32_t waitFlagType, uint32_t expectedFlag, 
					uint32_t* resultFlag)
{
	uint32_t result;
	
	uint32_t flags = expectedFlag;
	
	uint32_t st = enterCritical();
	
	result = eFlagGroupCheckExpected(eflaggroup, waitFlagType, &flags);
	
	// �����������
	if(!result) {
		*resultFlag = flags;
		leaveCritical(st);
		return NO_ERROR;
	}
	
	*resultFlag = flags;
	
	leaveCritical(st);
	
	return ERROR_RESOURCE_UNAVAILABLE;
}

// �뷨���ڲ���event��ʱ��(eventWait����)�������ð����ȼ�����
// ������ȡ�������ʱ��ȡ����һ���������ȼ���ߵ�һ��
// ��ô�˺����Ϳ���ֱ�Ӱ������ȼ�����������
void eFlagGroupPost(eFlagGroup_t* eFlagGroup, uint32_t flags, uint32_t isSet){
	if (flags == 0) return;
	if (isSet) {
		eFlagGroup->flags |= flags;
	} else {
		eFlagGroup->flags &= ~flags;
	}
	
	uint32_t schedFlag = 0;
	
	uint32_t st = enterCritical();
	
	for(listNode* node = eFlagGroup->event.waitlist.firstNode; node != &eFlagGroup->event.waitlist.headNode;) {
		task_t* task = getListNodeParent(node, task_t, linkNode);
		node = node->next;
		
		uint32_t flags = task->waitEventFlags;
		
		if (!eFlagGroupCheckExpected(eFlagGroup, task->waitEventFlagType, &flags)) {
			
			task->waitEventFlags = flags;
			
			eventWakeUpGivenTask(&eFlagGroup->event, task, NULL, NO_ERROR);
			
			if(task->priority < currentTask->priority) {
				schedFlag = 1;
			}
		}
	}
	
	if (schedFlag) {
		taskSched();
	}
	
	leaveCritical(st);
}


uint32_t eFlagGroupDestory(eFlagGroup_t* eFlagGroup) {
	uint32_t count;
	
	uint32_t st = enterCritical();
	
	
	if ((count = eventRemoveAllTask(&eFlagGroup->event, NULL, ERROR_DELETED)) > 0) {
		taskSched();
	}
	
	leaveCritical(st);
	
	return count;
}


eFlagGroupInfo_t eFlagGroupGetInfo(eFlagGroup_t* eFlagGroup) {
	eFlagGroupInfo_t info;
	
	uint32_t st = enterCritical();
	
	info.flags = eFlagGroup->flags;
	info.waitTaskNum = getListNodeNum(&eFlagGroup->event.waitlist);
	
	leaveCritical(st);
	
	return info;
}

// �˺���Ч�ʸ�����һ������
/**
void eFlagGroupGetInfo(eFlagGroup_t* eFlagGroup, eFlagGroupInfo_t* info) {
	uint32_t st = enterCritical();
	
	info->flags = eFlagGroup->flags;
	info->waitTaskNum = getListNodeNum(&eFlagGroup->event.waitlist);
	
	leaveCritical(st);
}
**/


#endif
