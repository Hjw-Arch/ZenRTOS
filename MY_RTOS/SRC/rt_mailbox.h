#ifndef RT_MAILBOX_H
#define RT_MAILBOX_H

#include "rt_event.h"

typedef struct _rt_mailbox {
	eventCtrlBlock_t event;
	uint32_t counter;  // 记录邮件数量
	uint32_t maxcount;	// 能够保存的最大消息数量
	uint32_t readPos;	// 读索引
	uint32_t writePos;	// 写索引
	void** messageBuffer;	// 消息缓存区
	
}mailBox_t;

void mailBoxInit(mailBox_t* mailbox, void** messageBuffer, uint32_t maxcount);


#endif

