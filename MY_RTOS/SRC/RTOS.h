#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>
#include "rtLib.h"
#include "lock.h"

#define NVIC_INT_CTRL				0xe000ed04	// �жϿ��Ƽ�״̬�Ĵ���ICSR
#define NVIC_PENDSVSET				0x10000000  //  pendsv�ж�ʹ��λ��ʹ��PendSVC�ж�
#define NVIC_SYSPRI2				0xe000ed22	//  PendSV�����ȼ��Ĵ���
#define NVIC_PENDSV_PRI				0xff  //  PendSV���ȼ�������λ������ȼ�

#define TIME_SLICE					20  //����ʱ��Ƭ

#define MEM32(addr)					*(volatile unsigned long*)(addr)
#define MEM8(addr)					*(volatile unsigned char*)(addr)


// ���������ջ������Ϊuint32
typedef uint32_t taskStack_t;

// ��������ṹ
typedef struct _t_Task {
	taskStack_t *stack;  // �����ջָ��
	
	uint32_t delayTicks; // ������ʱ���������ڵ�����ʱ����ʱÿSysTick�жϼ�һ
}task_t;

extern task_t* currentTask;
extern task_t* nextTask;
extern task_t* taskTable[2];


void runFirstTask(void);
void runFirstTask2(void);
void taskSwitch(void);


#endif
