#ifndef RT_TASK_H
#define RT_TASK_H

#include <stdint.h>
#include "rtConfig.h"

// ���������ջ������Ϊuint32
typedef uint32_t taskStack_t;

// ��������ṹ
typedef struct _t_Task {
	taskStack_t *stack;  // �����ջָ��
	
	uint32_t delayTicks; // ������ʱ���������ڵ�����ʱ����ʱÿSysTick�жϼ�һ
	
	uint32_t priority;
}task_t;

extern task_t _idleTask;
extern task_t* currentTask;
extern task_t* nextTask;
extern task_t* idleTask;
extern task_t* taskTable[RTOS_PRIORITY_COUNT];
extern taskStack_t idleTaskEnv[512];


void runFirstTask(void);
void runFirstTask2(void);
void taskSwitch(void);
void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack, uint32_t priority);
void taskSched(void);
void taskDelay (uint32_t ms);
void setSysTick(uint32_t ms);
void idleTaskEntry (void* param);

#endif
