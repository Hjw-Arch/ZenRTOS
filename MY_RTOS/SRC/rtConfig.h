#ifndef RTCONFIG_H
#define RTCONFIG_H

typedef enum _error {
	NO_ERROR = 0,
	ERROR_TIMEOUT = 1,
	ERROR_RESOURCE_UNAVAILABLE,
	ERROR_DELETED,
	ERROR_MAILBOX_FULL,
	ERROR_MEMBLOCK_FULL,
	ERROR_UNMATCHED_OWNER,
}rt_error;

#define RTOS_PRIORITY_COUNT			32

#define SYS_TICK					10  // 系统节拍，单位是ms， 即每SYS_TICK ms触发一次系统时钟中断

#define TIME_SLICE					10  // 任务时间片，单位是系统节拍

#define TASK_STACK_SIZE				1024  // 默认的任务堆栈大小

#define TIMER_TASK_STACK_SIZE		1024	// 定时器任务的堆栈大小
#define TIMER_TASK_PRIORITY			1		// 定时器任务的优先级，注意，不能和空闲任务的优先级一致，否则它两会按时间片交替运行

// 高实时性
#define HIGH_RT_MODE



#endif
