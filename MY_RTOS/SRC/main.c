#include "RTOS.h"
#include "ARMCM3.h"


task_t* idleTaskp;

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
	
	task->delayTicks = 0;
}

// ������Ⱥ�������������һ�����е��������ĸ�
void taskSched(void) {
	if (currentTask == idleTaskp) {
		if (taskTable[0]->delayTicks == 0) nextTask = taskTable[0];
		else if (taskTable[1]->delayTicks == 0) nextTask = taskTable[1];
		else return;
	}
	
	if (currentTask == taskTable[0] && taskTable[1]->delayTicks == 0) {
		nextTask = taskTable[1];
	} else if (currentTask == taskTable[1] && taskTable[0]->delayTicks == 0){
		nextTask = taskTable[0];
	} else if (taskTable[0]->delayTicks != 0 && taskTable[1] != 0) {
		nextTask = idleTaskp;
	}
	
	taskSwitch();
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
	if (ms < TIME_SLICE) ms = TIME_SLICE;
	
	currentTask->delayTicks = (ms + TIME_SLICE / 2) / TIME_SLICE; // ���������㷨
	
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


int task1Flag;
void task1Entry (void* param) {
	setSysTick(TIME_SLICE);
	while(1) {
		task1Flag = 0;
		taskDelay(20);
		task1Flag = 1;
		taskDelay(20);
	}
}

int task2Flag;
void task2Entry (void* param) {
	while(1) {
		task2Flag = 0;
		taskDelay(20);
		task2Flag = 1;
		taskDelay(20);
	}
}

task_t ttask1;
task_t ttask2;
task_t idleTask;

taskStack_t task1Env[1024];
taskStack_t task2Env[1024];
taskStack_t idleTaskEnv[512];


void idleTaskEntry (void* param) {
	while(1) ;
}

int main(){
	taskInit(&ttask1, task1Entry, (void*)0x1145, &task1Env[1024]);
	taskInit(&ttask2, task2Entry, (void*)0x1919, &task2Env[1024]);
	
	// idletask
	taskInit(&idleTask, idleTaskEntry, (void*)0, &idleTaskEnv[512]);
	
	taskTable[0] = &ttask1;
	taskTable[1] = &ttask2;
	
	nextTask = &ttask1;
	idleTaskp = &idleTask;
	
//	runFirstTask(); // ����֮�󲻻᷵�أ��·���return 0��ʵûʲô����
	
//	��idletask��ջ�����ã����������л�����������������ǰ��R4-R11��û���ã������������˷ѿռ䡣
//	ֻҪ��psp��currenttaskָ��һ�����õĵ�ַ�Ϳ�����
	__set_PSP((uint32_t)(&idleTaskEnv[256]));
	currentTask = (task_t*)(&idleTaskEnv[256]);		// ��currenttaskָ��idletaskջ�����������л�����ʹ�ã�û���ã������������˷ѿռ䡣
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;			// ����PendSVC�����ȼ�
	
	taskSwitch();
	
	return 0;
}

