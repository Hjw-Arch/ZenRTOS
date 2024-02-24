#include "rt_task.h"
#include "lock.h"
#include "ARMCM3.h"

Bitmap taskPriorityBitmap;

task_t _idleTask;

task_t* currentTask;
task_t* nextTask;
task_t* idleTask;
task_t* taskTable[RTOS_PRIORITY_COUNT];

taskStack_t idleTaskEnv[512];

listHead taskDelayedList;		// ��ʱ����


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
	task->state = TASK_STATUS_READY;	// ��ʼ״̬Ϊ����̬
	listNodeInit(&(task->delayNode)); // ��ʼ����ʱ���
	
	// ��task�������ȼ����У�������Ӧ��λͼ��λ
	taskTable[priority] = task;
	bitmapSet(&taskPriorityBitmap, priority);
}



// ������Ⱥ�������������һ�����е��������ĸ�
void taskSched(void) {
	uint32_t st = enterCritical();
	
	// �������������������0�����л�����
	if (schedLockCount > 0) {
		leaveCritical(st);
		return;
	}
	
	// Ѱ��������ȼ�������
	task_t* tempTask = getHighestReadyTask();
	
	// ���������ȼ��������ǵ�ǰ���������л���������ȼ�������������У������л�
	if (tempTask != currentTask) {
		nextTask = tempTask;
		taskSwitch();
	}
	
	leaveCritical(st);
}


// ��������������
void taskSched2Ready(task_t* task) {
	taskTable[task->priority] = task;
	bitmapSet(&taskPriorityBitmap, task->priority);
}

// ������Ӿ��������Ƴ�
void taskSched2Unready(task_t* task) {
	taskTable[task->priority] = NULL;
	bitmapClear(&taskPriorityBitmap, task->priority);
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
	
	uint32_t st = enterCritical();
	
	currentTask->delayTicks = (ms + TIME_SLICE / 2) / TIME_SLICE; // ���������㷨
	
	taskWait(currentTask);
	
	taskSched2Unready(currentTask);
	
	leaveCritical(st);
	
	taskSched();
}

void taskTimeSliceHandler() {
	// �����ƣ�����ֱ��ʹ����list�ڲ���Ԫ�ؽ��е�����û�н��з�װ������ʵ��һ���������������װһ��
	for (listNode* node = taskDelayedList.firstNode; node != &(taskDelayedList.headNode); node = node->next) {
		task_t *task = getListNodeParent(node, task_t, delayNode);
		if (--task->delayTicks == 0) {
			taskWakeUp(task);
			taskSched2Ready(task);
		}
	}
	
	taskSched();
}

// ����SysTick��ʱ�жϵ�ʱ�䣨�ж����ڣ������ʱ��Ƭ�� 
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


task_t* getHighestReadyTask(void) {
	return taskTable[bitmapGetFirstSet(&taskPriorityBitmap)];
}

// ��ʼ��������ʱ����
void taskDelayedListInit(void) {
	listHeadInit(&taskDelayedList);
}

void taskWait(task_t* task) {
	listNodeInsert2Head(&taskDelayedList, &task->delayNode);
	task->state |= TASK_STATUS_DELAY;
}

void taskWakeUp(task_t* task) {
	listRemove(&taskDelayedList, &task->delayNode);
	task->state &= ~TASK_STATUS_DELAY;
}
