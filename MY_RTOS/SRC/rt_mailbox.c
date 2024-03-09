#include "rt_mailbox.h"
#include "lock.h"

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
uint32_t mboxWait(mbox_t* mbox, void** msg, uint32_t waitTime) {
	uint32_t st = enterCritical();
	
	if (mbox->counter > 0) {
		--mbox->counter;
		*msg = mbox->messageBuffer[mbox->readPos];
		if (++mbox->readPos == mbox->maxcount) {
			mbox->readPos = 0;
		}
		leaveCritical(st);
		return NO_ERROR;
	}
	
	eventWait(&mbox->event, currentTask, NULL, TASK_STATUS_WAIT_MAILBOX, waitTime);
	taskSched();
	leaveCritical(st);
	
	*msg = currentTask->eventMsg;
	return currentTask->eventWaitResult;
}

// ��ȡһ���ʼ���û���ʼ�������
uint32_t mboxGetWithNoWait(mbox_t* mbox, void** msg) {
	uint32_t st = enterCritical();
	
	if (mbox->counter > 0) {
		--mbox->counter;
		*msg = mbox->messageBuffer[mbox->readPos];
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



