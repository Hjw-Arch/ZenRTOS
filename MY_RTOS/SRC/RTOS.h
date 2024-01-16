#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

#define NVIC_INT_CTRL				0xe000ed04	// �жϿ��Ƽ�״̬�Ĵ���ICSR
#define NVIC_PENDSVSET			0x10000000  // pendsv�ж�ʹ��λ
#define NVIC_SYSPRI2				0xe000ed22	// PendSV�����ȼ��Ĵ���
#define NVIC_PENDSV_PRI			0xff  // PendSV���ȼ�

#define MEM32(addr)					*(volatile unsigned long*)(addr)
#define MEM8(addr)					*(volatile unsigned char*)(addr)



// ���������ջ������Ϊuint32
typedef uint32_t taskStack_t;


typedef struct _t_Task {
		taskStack_t *stack;
}task_t;



#endif
