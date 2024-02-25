#ifndef RT_TASK_H
#define RT_TASK_H

#include <stdint.h>
#include "rtConfig.h"
#include "rtLib.h"


#define TASK_STATUS_READY		0				// ����״̬������̬
#define TASK_STATUS_DELAY		(1 << 1)		// ����״̬����ʱ̬
#define TASK_STATUS_SUSPEND		(1 << 2)		// ����״̬������̬


// ���������ջ������Ϊuint32
typedef uint32_t taskStack_t;

// ��������ṹ
typedef struct _t_Task {
	taskStack_t *stack;  // �����ջָ��
	
	uint32_t slice; // ʱ��Ƭ
	
	uint32_t delayTicks; // ������ʱ���������ڵ�����ʱ����ʱÿSysTick�жϼ�һ
	
	listNode linkNode;
	
	listNode delayNode; // ��ʱ���н��
	
	uint32_t state; // �����ʱ��״̬
	
	uint32_t priority; // ��������ȼ�
	
	uint32_t suspendCounter; // ���������
}task_t;

extern task_t _idleTask;
extern task_t* currentTask;
extern task_t* nextTask;
extern task_t* idleTask;
extern listHead taskTable[RTOS_PRIORITY_COUNT];
extern taskStack_t idleTaskEnv[512];

extern listHead taskDelayedList;

extern Bitmap taskPriorityBitmap;


void runFirstTask(void);
void runFirstTask2(void);
void taskSwitch(void);

void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack, uint32_t priority);
void taskSched(void);

void idleTaskEntry (void* param);
task_t* getHighestReadyTask(void);
void taskDelayedListInit(void);
void taskSched2Ready(task_t* task);
void taskSched2Unready(task_t* task);

void taskSched2Delay(task_t* task);
void taskSched2Undelay(task_t* task);

void taskSuspend(task_t* task);
void taskWakeUp(task_t* task);

#endif
