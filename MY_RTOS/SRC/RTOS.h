#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

#define NVIC_INT_CTRL				0xe000ed04	// 中断控制及状态寄存器ICSR
#define NVIC_PENDSVSET			0x10000000  //  pendsv中断使能位，使能PendSVC中断
#define NVIC_SYSPRI2				0xe000ed22	//  PendSV的优先级寄存器
#define NVIC_PENDSV_PRI			0xff  //  PendSV优先级，设置位最低优先级

#define MEM32(addr)					*(volatile unsigned long*)(addr)
#define MEM8(addr)					*(volatile unsigned char*)(addr)



// 定义任务堆栈的类型为uint32
typedef uint32_t taskStack_t;

// 定义任务结构
typedef struct _t_Task {
		taskStack_t *stack;  // 任务的栈指针
}task_t;

extern task_t* currentTask;
extern task_t* nextTask;
extern task_t* taskTable[2];


void runFirstTask(void);
void taskSwitch(void);


#endif
