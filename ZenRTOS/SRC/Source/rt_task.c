#include "rt_task.h"
#include "lock.h"
#include <string.h>
#include "rt_time.h"
#include "rt_hooks.h"
#if CORTEX_M4_FPU_OPENED == 1
#include "stm32f4xx.h"
#else
#include "ARMCM3.h"
#endif

Bitmap taskPriorityBitmap;

task_t* currentTask;
task_t* nextTask;

listHead taskTable[RTOS_PRIORITY_COUNT];

extern task_t* idleTask;


// �����ʼ��
void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack, uint32_t priority, uint32_t stackSize) {
	task->stackBase = stack;
	task->stackSize = stackSize;
	memset(stack, 0, stackSize);
	
	taskStack_t* stackTop = stack + stackSize / sizeof(taskStack_t);
	
	// �����ж�/�쳣ʱ�� Ӳ�����Զ���8���Ĵ���ѹջ��˳����xPSR��PC(R15)��LR(R14)��R12�Լ�R3-R0
#if CORTEX_M4_FPU_OPENED == 1
	stackTop -= 18;
#endif
	*(--stackTop) = (unsigned long)(1 << 24); // xPSR�е�24λ����T��־λ����Ϊ1���������ARMģʽ������CM3�ϲ�����
	*(--stackTop) = (unsigned long)entry;
	*(--stackTop) = (unsigned long)0x14;
	*(--stackTop) = (unsigned long)0x12;
	*(--stackTop) = (unsigned long)0x03;
	*(--stackTop) = (unsigned long)0x02;
	*(--stackTop) = (unsigned long)0x01;
	*(--stackTop) = (unsigned long)param;
	
	// �ֶ�����R11-R4�Ĵ���
	*(--stackTop) = (unsigned long)0x11;
	*(--stackTop) = (unsigned long)0x10;
	*(--stackTop) = (unsigned long)0x09;
	*(--stackTop) = (unsigned long)0x08;
	*(--stackTop) = (unsigned long)0x07;
	*(--stackTop) = (unsigned long)0x06;
	*(--stackTop) = (unsigned long)0x05;
	*(--stackTop) = (unsigned long)0x04;
	
		
	task->stackTop = stackTop;
	task->delayTicks = 0;
	task->priority = priority;
	task->state = TASK_STATUS_READY;	// ��ʼ״̬Ϊ����̬
	task->slice = TIME_SLICE;
	task->suspendCounter = 0;
	task->clean = NULL;
	task->cleanParam = NULL;
	task->requestDeleteFlag = 0;
	task->waitEvent = NULL;
	task->eventMsg = NULL;
	task->eventWaitResult = NO_ERROR;
	listNodeInit(&(task->delayNode)); // ��ʼ����ʱ���
	listNodeInit(&(task->linkNode)); // ��ʼ��������
	
	// ��task�������ȼ����У�������Ӧ��λͼ��λ
	taskSched2Ready(task);

#if FUNCTION_HOOKS_ENABLE == 1
	hooksTaskInit(task);
#endif

	if(currentTask != idleTask && currentTask->priority > task->priority) {
		taskSched();
	}
}


// ���ڵ��ô˺��������к��������������˺������ؼ���
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
		
#if FUNCTION_HOOKS_ENABLE == 1
		hooksTaskSwitch(currentTask, nextTask);
#endif
		
		taskSwitch();
	}
	
	leaveCritical(st);
}

void taskYield(void) {
	uint32_t st = enterCritical();
	
	// �������������������0�����л�����
	if (schedLockCount > 0) {
		leaveCritical(st);
		return;
	}
	
	// Ѱ��������ȼ�������
	uint32_t hightPriorityTask = bitmapGetFirstSet(&taskPriorityBitmap);

	
	if (hightPriorityTask == currentTask->priority) {
		listNode* node = listRemoveFirst(&taskTable[hightPriorityTask]);
		listNodeInsert2Tail(&taskTable[hightPriorityTask], node);
	}
	
	listNode* node = getFirstListNode(&taskTable[hightPriorityTask]);
	
	nextTask = getListNodeParent(node, task_t, linkNode);
	
	// ���������ȼ��������ǵ�ǰ���������л���������ȼ�������������У������л�

#if FUNCTION_HOOKS_ENABLE == 1
		hooksTaskSwitch(currentTask, nextTask);
#endif
	if (nextTask != currentTask) {
		taskSwitch();
	}

	leaveCritical(st);
}


task_t* getHighestReadyTask(void) {
	uint32_t hightPriorityTask = bitmapGetFirstSet(&taskPriorityBitmap);
	listNode* node = getFirstListNode(&taskTable[hightPriorityTask]);
	return (task_t*)getListNodeParent(node, task_t, linkNode);
}

// ��ʼ��������ʱ����
void taskDelayedListInit(void) {
	listHeadInit(&taskDelayedList);
}

// Ӧ������飬��������״̬�����������ʱ������ɾ��״̬�Ͳ�Ӧ�ý�����������
// ����������������������������̬
// ��ԭ�Ӳ���
void taskSched2Ready(task_t* task) {
	listNodeInsert2Tail(&taskTable[task->priority], &task->linkNode);
	task->state = TASK_STATUS_READY;
	bitmapSet(&taskPriorityBitmap, task->priority);
}

// ������Ӿ��������Ƴ�����������Ӿ���̬�Ƴ�
void taskSched2Unready(task_t* task) {
	
	if (!checkNodeIsInList(&taskTable[task->priority], &task->linkNode)) {
		return;
	}
	
	listRemove(&taskTable[task->priority], &task->linkNode);
	
	task->state &= ~TASK_STATUS_READY;
	
	if (getListNodeNum(&taskTable[task->priority]) == 0){
		bitmapClear(&taskPriorityBitmap, task->priority);
	}
}

// �����������ʱ���У�״̬����Ϊ��ʱ̬
void taskSched2Delay(task_t* task, uint32_t ms) {
	if (ms < SYS_TICK) ms = SYS_TICK;
	currentTask->delayTicks = (ms + SYS_TICK / 2) / SYS_TICK; // ���������㷨
	listNodeInsert2Head(&taskDelayedList, &task->delayNode);
	task->state |= TASK_STATUS_DELAY;
}

// �������Ƴ���ʱ���У�����ʱ̬���
void taskSched2Undelay(task_t* task) {
	
	if (!checkNodeIsInList(&taskDelayedList, &task->delayNode)) {
		return;
	}
	
	listRemove(&taskDelayedList, &task->delayNode);
	
	task->delayTicks = 0;
	
	task->state &= ~TASK_STATUS_DELAY;
}


// ���ܱ�������ã�Ҫ����
void taskSuspend(task_t* task) {
	uint32_t st = enterCritical();
	
	if (task->state & TASK_STATUS_DELAY) {
		leaveCritical(st);
		return;
	}
	
	if (task->suspendCounter++ == 0) {
		taskSched2Unready(task);
		task->state |= TASK_STATUS_SUSPEND;
		if (task == currentTask) {
			taskSched();
		}
	}
	
	leaveCritical(st);
}

// ���ѹ��������
// 2024.3.8�޸��˵����㷨�������ѵ��������ȼ����ڵ�ǰ����Ž����������
void taskWakeUp(task_t* task) {
	uint32_t st = enterCritical();
	
	if (task->state & TASK_STATUS_SUSPEND) {
		if (--task->suspendCounter == 0) {
			task->state &= ~TASK_STATUS_SUSPEND;
			taskSched2Ready(task);
			if (task->priority < currentTask->priority) {
				taskSched();
			}
		}
	}
	
	leaveCritical(st);
}

// �������������ص�����
void taskSetCleanCallFunc (task_t* task, void (*clean)(void* param), void* param) {
	task->clean = clean;
	task->cleanParam = param;
}

// ǿ��ɾ������
void taskForceDelete (task_t* task) {
	uint32_t st = enterCritical();
	
	if (task->state & TASK_STATUS_DELAY) {
		taskSched2Undelay(task);
	}else if (!(task->state & TASK_STATUS_SUSPEND)) {
		taskSched2Unready(task);
	}
	
	if (task->clean) {
		task->clean(task->cleanParam);
	}
	
	if (task == currentTask) {
		taskSched();
	}
	
	leaveCritical(st);
}

//�����������������ٽ���������һ����Ҫ

// ����ɾ������
void taskRequestDelete(task_t* task){
	task->requestDeleteFlag = 1;
}

// ��ѯ�Ƿ�����ɾ��
uint8_t taskIsRequestedDelete(task_t* task) {
	return task->requestDeleteFlag;
}

// ����ɾ���Լ�
void taskDeleteSelf(void) {
	uint32_t st = enterCritical();
	
	taskSched2Unready(currentTask);
	
	if (currentTask->clean) {
		currentTask->clean(currentTask->cleanParam);
	}
	
	taskSched();
	
	leaveCritical(st);
}

// �˴����ܿ��Լ����Ż�Ϊ����İ汾�������˴���taskinfo�͸���taskinfo�ĳɱ�
// �����ַ�ʽ�����ϱ��ϰ��
taskInfo_t taskGetInfo(task_t* task) {
	taskInfo_t taskinfo;
	taskStack_t* stackEnd;
	uint32_t st = enterCritical();
	
	taskinfo.priority = task->priority;
	taskinfo.slice = task->slice;
	taskinfo.state = task->state;
	taskinfo.suspendCounter = task->suspendCounter;
	taskinfo.stackSize = task->stackSize;
	
	taskinfo.stackRtFreeSize = (task->stackTop - task->stackBase) * sizeof(taskStack_t);
	
	taskinfo.stackMinFreeSize = 0;
	stackEnd = task->stackBase;
	
	while((*stackEnd++ == 0) && (stackEnd <= task->stackTop)) {
		taskinfo.stackMinFreeSize++;
	}
	
	taskinfo.stackMinFreeSize *= sizeof(taskStack_t);
	
	leaveCritical(st);
	
	return taskinfo;
}

/** �˰汾�������һ��������������Сһ��
void taskGetInfo(task_t* task, taskInfo_t* taskinfo) {
	taskStack_t* stackEnd;
	uint32_t st = enterCritical();
	
	taskinfo->priority = task->priority;
	taskinfo->slice = task->slice;
	taskinfo->state = task->state;
	taskinfo->suspendCounter = task->suspendCounter;

	taskinfo->stackRtFreeSize = (task->stackTop - task->stackBase) * sizeof(taskStack_t);
	
	taskinfo->stackMinFreeSize = 0;
	stackEnd = task->stackBase;
	
	while((*stackEnd++ == 0) && (stackEnd <= task->stackTop)) {
		taskinfo->stackMinFreeSize++;
	}
	
	taskinfo->stackMinFreeSize *= sizeof(taskStack_t);
	
	leaveCritical(st);
}
**/
