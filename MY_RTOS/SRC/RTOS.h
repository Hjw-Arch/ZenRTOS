#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

#define NVIC_INT_CTRL				0xe000ed04	// �жϿ��Ƽ�״̬�Ĵ���ICSR
#define NVIC_PENDSVSET			0x10000000  //  pendsv�ж�ʹ��λ��ʹ��PendSVC�ж�
#define NVIC_SYSPRI2				0xe000ed22	//  PendSV�����ȼ��Ĵ���
#define NVIC_PENDSV_PRI			0xff  //  PendSV���ȼ�������λ������ȼ�

#define MEM32(addr)					*(volatile unsigned long*)(addr)
#define MEM8(addr)					*(volatile unsigned char*)(addr)



// ���������ջ������Ϊuint32
typedef uint32_t taskStack_t;

// ��������ṹ
typedef struct _t_Task {
		taskStack_t *stack;  // �����ջָ��
}task_t;

extern task_t* currentTask;
extern task_t* nextTask;
extern task_t* taskTable[2];


void runFirstTask(void);
void taskSwitch(void);


#endif
