#ifndef RT_IDLETASK_H
#define RT_IDLETASK_H

#include "rt_task.h"

extern task_t _idleTask;
extern task_t* idleTask;
extern taskStack_t idleTaskEnv[TASK_STACK_SIZE];

// 供给时钟中断使用，检查CPU利用率
void checkCpuUsage(void);

// 获取利用率数据
float cpuGetUsage(void);


#endif
