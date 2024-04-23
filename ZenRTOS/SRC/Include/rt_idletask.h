#ifndef RT_IDLETASK_H
#define RT_IDLETASK_H

#include "rt_task.h"

extern task_t _idleTask;
extern task_t* idleTask;
extern taskStack_t idleTaskEnv[TASK_STACK_SIZE];

// ����ʱ���ж�ʹ�ã����CPU������
void checkCpuUsage(void);

// ��ȡ����������
float cpuGetUsage(void);


#endif
