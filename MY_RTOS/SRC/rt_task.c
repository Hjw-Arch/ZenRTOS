#include "rt_task.h"
#include "lock.h"
#include "ARMCM3.h"
#include "rtLib.h"

Bitmap taskPriorityBitmap;

task_t _idleTask;

task_t* currentTask;
task_t* nextTask;
task_t* idleTask;
task_t* taskTable[RTOS_PRIORITY_COUNT];

taskStack_t idleTaskEnv[512];



// �����ʼ��
void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack, uint32_t priority) {
	// �����ж�/�쳣ʱ�� Ӳ�����Զ���8���Ĵ���ѹջ��˳����xPSR��PC(R15)��LR(R14)��R12�Լ�R3-R0
	*(--stack) = (unsigned long)(1 << 24); // xPSR�е�24λ����T��־λ����Ϊ1���������ARMģʽ������CM3�ϲ�����
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x03;
	*(--stack) = (unsigned long)0x02;
	*(--stack) = (unsigned long)0x01;
	*(--stack) = (unsigned long)param;
	
	// �ֶ�����R11-R4�Ĵ���
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x09;
	*(--stack) = (unsigned long)0x08;
	*(--stack) = (unsigned long)0x07;
	*(--stack) = (unsigned long)0x06;
	*(--stack) = (unsigned long)0x05;
	*(--stack) = (unsigned long)0x04;
	
		
	task->stack = stack;
	task->delayTicks = 0;
	task->priority = priority;
	
	// ��task�������ȼ����У�������Ӧ��λͼ��λ
	taskTable[priority] = task;
	bitmapSet(&taskPriorityBitmap, priority);
}

// ������Ⱥ�������������һ�����е��������ĸ�
void taskSched(void) {
	uint32_t st = enterCritical();
	
	if (schedLockCount > 0) {
		leaveCritical(st);
		return;
	}
	
	if (currentTask == idleTask) {
		if (taskTable[0]->delayTicks == 0) nextTask = taskTable[0];
		else if (taskTable[1]->delayTicks == 0) nextTask = taskTable[1];
		else {
			leaveCritical(st);
			return;
		}
	}
	
	if (currentTask == taskTable[0] && taskTable[1]->delayTicks == 0) {
		nextTask = taskTable[1];
	} else if (currentTask == taskTable[1] && taskTable[0]->delayTicks == 0){
		nextTask = taskTable[0];
	} else if (taskTable[0]->delayTicks != 0 && taskTable[1] != 0) {
		nextTask = idleTask;
	}
	
	taskSwitch();
	
	leaveCritical(st);
}

// Ҫʵ��������ʱ����Ҫʹ�ö�ʱ��������ÿ�������䱸һ����ʱ�����У�����Ӳ��ֻ��һ����ʱ�������������ܶ�
// ��˿�������SysTick���Ӳ����ʱ����ʵ����ʱ��
// ��ΪSysTick�����Դ����жϣ���˿������������Ϊ������������ʱ����ʱ�䵥λ
// ÿ����һ��SysTick�ͽ���ʱ����ֵ-1����
// �����ʱ���Ķ�ʱʱ�䶼��SysTick�жϵı���
// �жϴ���Ҳ��Ҫʱ�䣬����̫Ƶ�������ᵼ��ϵͳ���л��������������ռ��̫����Դ����ĩ���ã�һ������Ϊ10ms-100ms�Ϳ���


// ��ʱ����ʱ���Ȳ���׼ȷ��Ҫע��ʹ�ó��� eg����ʱһ����λ��������SysTick�м俪ʼ��ʱ����ôֻ����ʱ���SysTick�ж����ڡ�
// �����ڽ�Ҫ������ʱ���жϵ�ʱ�����˸��߼�����жϣ��ᵼ����ʱʱ��䳤

void taskDelay (uint32_t ms) {
//  ���ڱ������費��Ҫ��������
	if (ms < TIME_SLICE) ms = TIME_SLICE;
	
//	uint32_t st = enterCritical();
	
	currentTask->delayTicks = (ms + TIME_SLICE / 2) / TIME_SLICE; // ���������㷨
//	leaveCritical(st);
	
	taskSched();
}

void taskTimeSliceHandler() {
	for (int i = 0; i < 2; ++i) {
		if (taskTable[i]->delayTicks > 0) {
			taskTable[i]->delayTicks--;
		}
	}
	
	taskSched();
}

// ����SysTick��ʱ�жϵ�ʱ�䣨�ж����ڣ������ʱ��Ƭ��
// 
void setSysTick(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; // �趨Ԥ��ֵ
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);  //�趨���ȼ�
	SysTick->VAL = 0; //�趨��ʱ��ֵ
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; //ʹ���ں˶�ʱ����������ʱ�жϣ�ʹ��SysTick
}

void SysTick_Handler() {
	taskTimeSliceHandler();
}


void idleTaskEntry (void* param) {
	while(1) ;
}
