#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "rt_event.h"


// �ź����Ķ���
typedef struct _semaphore {
	eventCtrlBlock_t event;
	uint32_t counter;
	uint32_t maxCount;
}sem_t;


// �ź�����״̬�ṹ�����ڱ����ź�����״̬
typedef struct _seminfo {
	uint32_t counter;
	uint32_t maxcount;
	uint32_t waitTaskNum;
}semInfo_t;


void semInit(sem_t* semaphore, uint32_t initCount, uint32_t maxCount);
uint32_t semWait(sem_t* semaphore, uint32_t waitTime);
uint32_t semGetWithNoWait(sem_t* semaphore);
void semPost(sem_t* semaphore);
uint32_t semDestory(sem_t* semaphore);
semInfo_t semGetInfo(sem_t* semaphore);
// void semGetInfo(sem_t* semaphore, semInfo_t* info);	//�������һ��������һ�ָ�Ϊ��Ч��ʵ�ַ�ʽ



#endif
