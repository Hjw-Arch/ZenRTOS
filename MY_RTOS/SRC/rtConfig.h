#ifndef RTCONFIG_H
#define RTCONFIG_H

#define RTOS_PRIORITY_COUNT			32

#define SYS_TICK					10  // 系统节拍，单位是ms

#define TIME_SLICE					10  // 任务时间片，单位是系统节拍

#define TASK_STACK_SIZE				1024  // 默认的任务堆栈大小


#define HIGH_RT_MODE



#endif
