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

// 高实时性模式
#define HIGH_RT_MODE				1		// 开启高实时性模式，在使用下面的一些功能时将任务阻塞，在唤醒时将优先唤醒高优先级任务，有可能导致低优先级任务长时间得不到运行

#define	FUNCTION_SEMAPHORE_ENABLE	1		// 信号量功能
#define	FUNCTION_MUTEX_EBABLE		1		// 互斥信号量
#define	FUNCTION_EFLAGGROUP_EBABLE	1		// 事件标志组
#define	FUNCTION_MBOX_ENABLE		1		// 邮箱
#define	FUNCTION_MBLOCK_ENABLE		1		// 存储块
#define	FUNCTION_SOFTTIMER_ENABLE	1		// 软定时器			此功能要求信号量必须开启
#define	FUNCTION_CPUUSAGE_ENABLE	1		// CPU使用率统计	关闭此功能，打开hooks功能并在hooksCpuIdle实现低功耗模式
#define FUNCTION_HOOKS_ENABLE		0		// 钩子函数


#endif
