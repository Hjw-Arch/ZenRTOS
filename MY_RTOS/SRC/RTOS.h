#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

#define NVIC_INT_CTRL				0xe000ed04	// 中断控制及状态寄存器ICSR
#define NVIC_PENDSVSET			0x10000000  // pendsv中断使能位
#define NVIC_SYSPRI2				0xe000ed22	// PendSV的优先级寄存器
#define NVIC_PENDSV_PRI			0xff  // PendSV优先级

#define MEM32(addr)					*(volatile unsigned long*)(addr)
#define MEM8(addr)					*(volatile unsigned char*)(addr)



// 定义任务堆栈的类型为uint32
typedef uint32_t taskStack_t;


typedef struct _t_Task {
		taskStack_t *stack;
}task_t;



#endif
