#ifndef RT_HOOKS_H
#define RT_HOOKS_H

#include "rt_task.h"

#if LOW_POWER_MODE == 1
extern uint8_t isenableLowPowerMode;

#define	 enableLowPower			isenableLowPowerMode = 1;
#define	 disableLowPower		isenableLowPowerMode = 0;
#define  LowPowerMode			isenableLowPowerMode
#endif

void hooksCpuIdle(void);
void hooksSysTick(void);
void hooksTaskSwitch(task_t* from, task_t* to);
void hooksTaskInit(task_t* task);


#endif
