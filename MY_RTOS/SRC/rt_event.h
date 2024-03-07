#ifndef RT_EVENT_H
#define RT_EVENT_H

#include "rtLib.h"
#include "rt_task.h"

typedef enum _eventType {
	eventTypeUnknown,
}eventType_t;

// 事件控制块的定义
typedef struct _eventCtrlBlock_t {
	eventType_t type;
	listHead waitlist;
}eventCtrlBlock_t;


void eventInit(eventCtrlBlock_t* ecb, eventType_t type);
void eventWait(eventCtrlBlock_t* event, task_t* task, void* msg, uint32_t state, uint32_t timeout);
task_t* eventWakeUp(eventCtrlBlock_t* event, void* msg, uint32_t result);
void eventRemoveTask(task_t* task, void* msg, uint32_t result);
uint32_t eventRemoveAllTask(eventCtrlBlock_t* event, void* msg, uint32_t result);
uint32_t eventGetWaitNum(eventCtrlBlock_t* event);



#endif
