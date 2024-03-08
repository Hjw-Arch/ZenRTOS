#ifndef RT_MAILBOX_H
#define RT_MAILBOX_H

#include "rt_event.h"

typedef struct _rt_mailbox {
	eventCtrlBlock_t event;
	uint32_t counter;  // ��¼�ʼ�����
	uint32_t maxcount;	// �ܹ�����������Ϣ����
	uint32_t readPos;	// ������
	uint32_t writePos;	// д����
	void** messageBuffer;	// ��Ϣ������
	
}mailBox_t;

void mailBoxInit(mailBox_t* mailbox, void** messageBuffer, uint32_t maxcount);


#endif

