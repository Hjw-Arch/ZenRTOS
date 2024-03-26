#ifndef RT_MUTEX_H
#define RT_MUTEX_H

#include "rt_event.h"

typedef struct _mutex {
	eventCtrlBlock_t event;
	
	uint32_t lockedCount; // ��������
	
	uint32_t ownerOriginalPriority; // ������ӵ���ߵ�ԭʼ���ȼ�
	
	task_t* owner;	// ��������ӵ����
}mutex_t;




#endif
