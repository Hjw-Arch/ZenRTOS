#ifndef RT_HOOKS_H
#define RT_HOOKS_H

#include "rt_task.h"

void hooksCpuIdle(void);
void hooksSysTick(void);
void hooksTaskSwitch(task_t* from, task_t* to);
void hooksTaskInit(task_t* task);


#endif
