#ifndef RT_EVENT_H
#define RT_EVENT_H

#include "rtLib.h"
#include "rt_task.h"

typedef enum _eventType {
	EVENT_TYPE_UNKNOWN,
	EVENT_TYPE_SEMAPHORE,
	
}eventType_t;

// �¼����ƿ�Ķ���
// ���type�е㼦�ߣ������費��Ҫȥ��
typedef struct _eventCtrlBlock_t {
	eventType_t type;
	listHead waitlist;
}eventCtrlBlock_t;


void eventInit(eventCtrlBlock_t* ecb, eventType_t type);
void eventWait(eventCtrlBlock_t* event, task_t* task, void* msg, uint32_t state, uint32_t waitTime);
task_t* eventWakeUp(eventCtrlBlock_t* event, void* msg, uint32_t result);
void eventRemoveTask(task_t* task, void* msg, uint32_t result);
uint32_t eventRemoveAllTask(eventCtrlBlock_t* event, void* msg, uint32_t result);
uint32_t eventGetWaitNum(eventCtrlBlock_t* event);



#endif
