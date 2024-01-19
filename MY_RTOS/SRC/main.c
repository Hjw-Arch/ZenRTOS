#include "RTOS.h"
#include "ARMCM3.h"


void triggerPenderSVC (void) {
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

// �����ʼ��
void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack) {
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
}

void taskSched(void) {
	if (currentTask == *taskTable) {
		nextTask = *(taskTable + 1);
	} else {
		nextTask = *taskTable;
	}
	
	taskSwitch();
}

// ����SysTick��ʱʱ��
void setSysTick(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; // �趨Ԥ��ֵ
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);  //�趨���ȼ�
	SysTick->VAL = 0; //�趨��ʱ��ֵ
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; //ʹ���ں˶�ʱ����������ʱ�жϣ�ʹ��SysTick
}

void SysTick_Handler() {
	taskSched();
}


void delay(int count) {
	while (--count) ;
}

task_t ttask1;
task_t ttask2;

taskStack_t task1Env[1024];
taskStack_t task2Env[1024];

int task1Flag;
void task1Entry (void* param) {
	setSysTick(10);
	while(1) {
		task1Flag = 0;
		delay(100);
		task1Flag = 1;
		delay(100);
	}
}

int task2Flag;
void task2Entry (void* param) {
	while(1) {
		task2Flag = 0;
		delay(100);
		task2Flag = 1;
		delay(100);
	}
}

int main(){
	
	taskInit(&ttask1, task1Entry, (void*)0x1145, &task1Env[1024]);
	taskInit(&ttask2, task2Entry, (void*)0x1919, &task2Env[1024]);
	
	taskTable[0] = &ttask1;
	taskTable[1] = &ttask2;
	
	nextTask = &ttask1;
	
	runFirstTask(); // ����֮�󲻻᷵�أ��·���return 0��ʵûʲô����
	
	return 0;
}

