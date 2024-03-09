#include "rt_mailbox.h"
#include "lock.h"

// 邮箱初始化
void mboxInit(mbox_t* mailbox, void** messageBuffer, uint32_t maxcount) {
	eventInit(&mailbox->event, EVENT_TYPE_MAILBOX);
	
	mailbox->messageBuffer = messageBuffer;
	mailbox->readPos = 0;
	mailbox->writePos = 0;
	mailbox->counter = 0;
	mailbox->maxcount = maxcount;
}

// 读取一个邮件，如果没有邮件就将任务阻塞在此
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

// 读取一个邮件，没有邮件就拉到
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

// 发送一个邮件，如果有任务在等待邮件，就直接给其中一个任务
// 如果邮箱已经满了，此邮件将被丢弃
// 否则存储在邮箱内
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



