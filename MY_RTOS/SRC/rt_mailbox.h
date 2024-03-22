#ifndef RT_MAILBOX_H
#define RT_MAILBOX_H

#include "rt_event.h"

#define PRIORITY_NORMAL		0
#define PRIORITY_HIGH		1

typedef struct _rt_mailbox {

	eventCtrlBlock_t event;
	uint32_t counter;  // ��¼�ʼ�����
	uint32_t maxcount;	// �ܹ�����������Ϣ����
	uint32_t readPos;	// ������
	uint32_t writePos;	// д����
	void** messageBuffer;	// ��Ϣ������
}mbox_t;


typedef struct _mboxInfo {
	uint32_t maxcount;	// ��������
	uint32_t mailNum;	// ��ǰ�ʼ�����
	uint32_t waitTaskNum;	// ��������
}mboxInfo_t;

void mboxInit(mbox_t* mailbox, void** messageBuffer, uint32_t maxcount);
uint32_t mboxWait(mbox_t* mbox, void* msg, uint32_t waitTime);
uint32_t mboxGetWithNoWait(mbox_t* mbox, void* msg);
uint32_t mboxPost(mbox_t* mbox, void* msg, uint32_t isHighPriority);
void mboxFlush(mbox_t* mbox);
uint32_t mboxDestory(mbox_t* mbox);
mboxInfo_t mboxGetInfo(mbox_t* mbox);
// void mboxGetInfo(mbox_t* mbox, mboxInfo_t* info);	// �������һ������������С


#endif

