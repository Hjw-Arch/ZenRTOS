#include "rt_mailbox.h"
#include "lock.h"

#if FUNCTION_MBOX_ENABLE == 1

// �����ʼ��
void mboxInit(mbox_t* mailbox, void** messageBuffer, uint32_t maxcount) {
	eventInit(&mailbox->event, EVENT_TYPE_MAILBOX);
	
	mailbox->messageBuffer = messageBuffer;
	mailbox->readPos = 0;
	mailbox->writePos = 0;
	mailbox->counter = 0;
	mailbox->maxcount = maxcount;
}

// ��ȡһ���ʼ������û���ʼ��ͽ����������ڴ�
uint32_t mboxWait(mbox_t* mbox, void* msg, uint32_t waitTime) {
	uint32_t st = enterCritical();
	
	// �������ֶ���ָ����Ϊ������warning��������������Ż���msg
	if (mbox->counter > 0) {
		--mbox->counter;
		*(void**)msg = mbox->messageBuffer[mbox->readPos];	// ����Ӹ�void*��Ϊ������warning
		if (++mbox->readPos == mbox->maxcount) {
			mbox->readPos = 0;
		}
		leaveCritical(st);
		return NO_ERROR;
	}
	
	eventWait(&mbox->event, currentTask, NULL, TASK_STATUS_WAIT_MAILBOX, waitTime);
	taskSched();
	leaveCritical(st);
	
	*(void**)msg = currentTask->eventMsg;
	return currentTask->eventWaitResult;
}

// ��ȡһ���ʼ���û���ʼ�������
uint32_t mboxGetWithNoWait(mbox_t* mbox, void* msg) {
	uint32_t st = enterCritical();
	
	if (mbox->counter > 0) {
		--mbox->counter;
		*(void**)msg = mbox->messageBuffer[mbox->readPos];
		if (++mbox->readPos == mbox->maxcount) {
			mbox->readPos = 0;
		}
		leaveCritical(st);
		return NO_ERROR;
	}
	
	leaveCritical(st);
	return ERROR_RESOURCE_UNAVAILABLE;
}

// ����һ���ʼ�������������ڵȴ��ʼ�����ֱ�Ӹ�����һ������
// ��������Ѿ����ˣ����ʼ���������
// ����洢��������
uint32_t mboxPost(mbox_t* mbox, void* msg, uint32_t isHighPriority) {
	uint32_t st = enterCritical();
	
	if (eventGetWaitNum(&mbox->event) > 0) {
		task_t* task = eventWakeUp(&mbox->event, (void*)msg, NO_ERROR);
		if (task->priority < currentTask->priority) {
			taskSched();
		}
		leaveCritical(st);
		return NO_ERROR;
	}
	
	if (mbox->counter >= mbox->maxcount) {
		leaveCritical(st);
		return ERROR_MAILBOX_FULL;
	}
	
	if (isHighPriority) {
		if (mbox->readPos == 0) {
			mbox->readPos = mbox->maxcount - 1;
			mbox->messageBuffer[mbox->readPos] = msg;
		}else {
			mbox->messageBuffer[--mbox->readPos] = msg;
		}
	}else {
		mbox->messageBuffer[mbox->writePos] = msg;
		if (++mbox->writePos == mbox->maxcount) {
			mbox->writePos = 0;
		}
	}
	
	++mbox->counter;
	leaveCritical(st);
	
	return NO_ERROR;
}

// ����ʼ�
// �鿴�Ƿ��������ڴ˵ȴ��ʼ������û�о��������
void mboxFlush(mbox_t* mbox) {
	uint32_t st = enterCritical();
	
	// ���û�������ڴ˵ȴ�����������գ����ﲻ����counter>0���жϣ���Ϊcounter==0��ʱ���޷��ж��Ƿ��������ڴ˵ȴ�
	if (eventGetWaitNum(&mbox->event) == 0) {
		mbox->counter = 0;
		mbox->readPos = 0;
		mbox->writePos = 0;
	}
	
	leaveCritical(st);
}

// �ݻ����䣨���������������ã��ڴ�û�еõ��ͷţ���ʹ��destory֮���ǿ��Լ���ʹ�õģ�
uint32_t mboxDestory(mbox_t* mbox) {
	uint32_t st = enterCritical();
	
	uint32_t count = eventRemoveAllTask(&mbox->event, NULL, ERROR_DELETED);
	
	mbox->counter = 0;
	mbox->readPos = 0;
	mbox->writePos = 0;
	
	if (count) {
		taskSched();
	}
	
	leaveCritical(st);
	
	return count;
}


// ��ȡ����״̬��Ϣ
mboxInfo_t mboxGetInfo(mbox_t* mbox) {
	mboxInfo_t info;
	
	uint32_t st = enterCritical();
	
	info.mailNum = mbox->counter;
	info.maxcount = mbox->maxcount;
	info.waitTaskNum = eventGetWaitNum(&mbox->event);
	
	leaveCritical(st);
	
	return info;
}

// �������һ������������С������һ�����������ϱ��ϰ��
/**
void mboxGetInfo(mbox_t* mbox, mboxInfo_t* info) {
	uint32_t st = enterCritical();
	
	info->mailNum = mbox->counter;
	info->maxcount = mbox->maxcount;
	info->waitTaskNum = eventGetWaitNum(&mbox->event);
	
	leaveCritical(st);
}
**/


#endif
