#ifndef RT_MAILBOX_H
#define RT_MAILBOX_H

#include "rt_event.h"

#define PRIORITY_NORMAL		0
#define PRIORITY_HIGH		1

typedef struct _rt_mailbox {

	eventCtrlBlock_t event;
	uint32_t counter;  // 记录邮件数量
	uint32_t maxcount;	// 能够保存的最大消息数量
	uint32_t readPos;	// 读索引
	uint32_t writePos;	// 写索引
	void** messageBuffer;	// 消息缓存区
}mbox_t;


typedef struct _mboxInfo {
	uint32_t maxcount;	// 邮箱容量
	uint32_t mailNum;	// 当前邮件数量
	uint32_t waitTaskNum;	// 任务数量
}mboxInfo_t;

void mboxInit(mbox_t* mailbox, void** messageBuffer, uint32_t maxcount);
uint32_t mboxWait(mbox_t* mbox, void* msg, uint32_t waitTime);
uint32_t mboxGetWithNoWait(mbox_t* mbox, void* msg);
uint32_t mboxPost(mbox_t* mbox, void* msg, uint32_t isHighPriority);
void mboxFlush(mbox_t* mbox);
uint32_t mboxDestory(mbox_t* mbox);
mboxInfo_t mboxGetInfo(mbox_t* mbox);
// void mboxGetInfo(mbox_t* mbox, mboxInfo_t* info);	// 相较于上一个函数开销更小


#endif

