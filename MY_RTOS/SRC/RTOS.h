#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>
#include "rtLib.h"
#include "lock.h"
#include "rtConfig.h"
#include "rt_task.h"

#define NVIC_INT_CTRL				0xe000ed04	// �жϿ��Ƽ�״̬�Ĵ���ICSR
#define NVIC_PENDSVSET				0x10000000  //  PendSVC�ж�ʹ��λ��ʹ��PendSVC�ж�
#define NVIC_SYSPRI2				0xe000ed22	//  PendSVC�����ȼ��Ĵ���
#define NVIC_PENDSV_PRI				0xff  //  PendSV���ȼ�������λ������ȼ�

#define MEM32(addr)					*(volatile unsigned long*)(addr)
#define MEM8(addr)					*(volatile unsigned char*)(addr)


#endif
