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

// 高实时性模式
#define HIGH_RT_MODE				1		// 开启高实时性模式，在使用下面的一些功能时将任务阻塞，在唤醒时将优先唤醒高优先级任务，有可能导致低优先级任务长时间得不到运行

#define	FUNCTION_SEMAPHORE_ENABLE	1		// 信号量功能
#define	FUNCTION_MUTEX_EBABLE		1		// 互斥信号量
#define	FUNCTION_EFLAGGROUP_EBABLE	1		// 事件标志组
#define	FUNCTION_MBOX_ENABLE		1		// 邮箱
#define	FUNCTION_MBLOCK_ENABLE		1		// 存储块
#define	FUNCTION_SOFTTIMER_ENABLE	1		// 软定时器			此功能要求信号量必须开启
#define	FUNCTION_CPUUSAGE_ENABLE	1		// CPU使用率统计	低功耗模式下此功能无效
#define FUNCTION_HOOKS_ENABLE		1		// 钩子函数
#define LOW_POWER_MODE				1		// 低功耗模式，开启此模式的前置条件为开启hooks功能

#define CORTEX_M4_FPU_OPENED		1		// cortex-M4内核开启FPU
// #define INIT_SIZE					16		// 若开启FPU，则改为34

#define MONITOR_FUNCTION




// 判断剪裁合理性
#if FUNCTION_SOFTTIMER_ENABLE == 1
#if FUNCTION_SEMAPHORE_ENABLE == 0
#error "Soft timer requests function semaphore must be enabled"
#endif
#endif

#if LOW_POWER_MODE == 1
#if FUNCTION_HOOKS_ENABLE == 0
#error "Low-power mode requests function hooks must be enabled"
#endif
#endif

#endif
