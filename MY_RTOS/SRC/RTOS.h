#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>
#include "rtLib.h"
#include "lock.h"
#include "rtConfig.h"
#include "rt_task.h"

#define NVIC_INT_CTRL				0xe000ed04	// 中断控制及状态寄存器ICSR
#define NVIC_PENDSVSET				0x10000000  //  PendSVC中断使能位，使能PendSVC中断
#define NVIC_SYSPRI2				0xe000ed22	//  PendSVC的优先级寄存器
#define NVIC_PENDSV_PRI				0xff  //  PendSV优先级，设置位最低优先级

#define MEM32(addr)					*(volatile unsigned long*)(addr)
#define MEM8(addr)					*(volatile unsigned char*)(addr)


#endif
