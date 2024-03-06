#include "rt_task.h"
#include "lock.h"
#include "ARMCM3.h"

Bitmap taskPriorityBitmap;

task_t _idleTask;

task_t* currentTask;
task_t* nextTask;
task_t* idleTask;
listHead taskTable[RTOS_PRIORITY_COUNT];

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
	task->state = TASK_STATUS_READY;	// ��ʼ״̬Ϊ����̬
	task->slice = TIME_SLICE;
	task->suspendCounter = 0;
	task->clean = NULL;
	task->cleanParam = NULL;
	task->requestDeleteFlag = 0;
	listNodeInit(&(task->delayNode)); // ��ʼ����ʱ���
	listNodeInit(&(task->linkNode)); // ��ʼ��������
	
	// ��task�������ȼ����У�������Ӧ��λͼ��λ
	taskSched2Ready(task);
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
		taskSwitch();
	}
	
	leaveCritical(st);
}


void idleTaskEntry (void* param) {
	while(1) ;
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

// ����������������������������̬
void taskSched2Ready(task_t* task) {
	listNodeInsert2Tail(&taskTable[task->priority], &task->linkNode);
	bitmapSet(&taskPriorityBitmap, task->priority);
}

// ������Ӿ��������Ƴ�����������Ӿ���̬�Ƴ�
void taskSched2Unready(task_t* task) {
	
	if (!checkNodeIsInList(&taskTable[task->priority], &task->linkNode)) {
		return;
	}
	
	listRemove(&taskTable[task->priority], &task->linkNode);
	
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
void taskWakeUp(task_t* task) {
	uint32_t st = enterCritical();
	
	if (task->state & TASK_STATUS_SUSPEND) {
		if (--task->suspendCounter == 0) {
			task->state &= ~TASK_STATUS_SUSPEND;
			taskSched2Ready(task);
			taskSched();
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
taskInfo_t getTaskInfo(task_t* task) {
	taskInfo_t taskinfo;
	
	uint32_t st = enterCritical();
	
	taskinfo.priority = task->priority;
	taskinfo.slice = task->slice;
	taskinfo.state = task->state;
	taskinfo.suspendCounter = task->suspendCounter;
	
	leaveCritical(st);
	
	return taskinfo;
}

/** �˰汾�������һ��������������Сһ��
void getTaskInfo(task_t* task, taskInfo_t* taskinfo) {
	
	uint32_t st = enterCritical();
	
	taskinfo->priority = task->priority;
	taskinfo->slice = task->slice;
	taskinfo->state = task->state;
	taskinfo->suspendCounter = task->suspendCounter;
	
	leaveCritical(st);
}
**/
