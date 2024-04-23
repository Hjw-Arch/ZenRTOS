#include "rt_mailbox.h"
#include "lock.h"

#if FUNCTION_MBOX_ENABLE == 1

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
uint32_t mboxWait(mbox_t* mbox, void* msg, uint32_t waitTime) {
	uint32_t st = enterCritical();
	
	// 创造这种二级指针是为了消除warning，否则编译器会优化掉msg
	if (mbox->counter > 0) {
		--mbox->counter;
		*(void**)msg = mbox->messageBuffer[mbox->readPos];	// 这里加个void*是为了消除warning
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

// 读取一个邮件，没有邮件就拉到
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

// 清空邮件
// 查看是否还有任务在此等待邮件，如果没有就清空邮箱
void mboxFlush(mbox_t* mbox) {
	uint32_t st = enterCritical();
	
	// 如果没有任务在此等待，将邮箱清空，这里不能用counter>0来判断，因为counter==0的时候无法判断是否有任务在此等待
	if (eventGetWaitNum(&mbox->event) == 0) {
		mbox->counter = 0;
		mbox->readPos = 0;
		mbox->writePos = 0;
	}
	
	leaveCritical(st);
}

// 摧毁邮箱（叫重置邮箱或许更好，内存没有得到释放，即使的destory之后还是可以继续使用的）
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


// 获取邮箱状态信息
mboxInfo_t mboxGetInfo(mbox_t* mbox) {
	mboxInfo_t info;
	
	uint32_t st = enterCritical();
	
	info.mailNum = mbox->counter;
	info.maxcount = mbox->maxcount;
	info.waitTaskNum = eventGetWaitNum(&mbox->event);
	
	leaveCritical(st);
	
	return info;
}

// 相较于上一个函数开销更小，但上一个函数更符合编程习惯
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
