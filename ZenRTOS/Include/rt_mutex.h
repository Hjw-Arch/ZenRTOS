#ifndef RT_MUTEX_H
#define RT_MUTEX_H

#include "rt_event.h"

typedef struct _mutex {
	eventCtrlBlock_t event;
	
	uint32_t lockedCount; // ��������
	
	uint32_t ownerOriginalPriority; // ������ӵ���ߵ�ԭʼ���ȼ�
	
	task_t* owner;	// ��������ӵ����
}mutex_t;

typedef struct _mutexinfo {
	uint32_t lockedCount;
	task_t* owner;
	uint32_t ownerOriginalPriority;
	uint32_t inheritedPriority;	//�̳����ȼ�����ӵ���ߵ�ǰʵ�ʵ����ȼ�
	uint32_t waitTaskNum;
}mutexInfo_t;

void mutexInit(mutex_t* mutex);
uint32_t mutexWait(mutex_t* mutex, uint32_t waitTime);
uint32_t mutexGetWithNoWait(mutex_t* mutex);
uint32_t mutexPost(mutex_t* mutex);
uint32_t mutexDestory(mutex_t* mutex);
mutexInfo_t mutexGetInfo(mutex_t* mutex);
// void mutexGetInfo(mutex_t* mutex, mutexInfo_t* info); //�������һ����������Ч��д��

#endif
