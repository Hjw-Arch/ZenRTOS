#include "rt_task.h"
#include "rtLib.h"
#include "lock.h"
#include "rt_event.h"

listHead taskDelayedList;		// ��ʱ����

// Ҫʵ��������ʱ����Ҫʹ�ö�ʱ��������ÿ�������䱸һ����ʱ�����У�����Ӳ��ֻ��һ����ʱ�������������ܶ�
// ��˿�������SysTick���Ӳ����ʱ����ʵ����ʱ��
// ��ΪSysTick�����Դ����жϣ���˿������������Ϊ������������ʱ����ʱ�䵥λ
// ÿ����һ��SysTick�ͽ���ʱ����ֵ-1����
// �����ʱ���Ķ�ʱʱ�䶼��SysTick�жϵı���
// �жϴ���Ҳ��Ҫʱ�䣬����̫Ƶ�������ᵼ��ϵͳ���л��������������ռ��̫����Դ����ĩ���ã�һ������Ϊ10ms-100ms�Ϳ���


// ��ʱ����ʱ���Ȳ���׼ȷ��Ҫע��ʹ�ó��� eg����ʱһ����λ��������SysTick�м俪ʼ��ʱ����ôֻ����ʱ���SysTick�ж����ڡ�
// �����ڽ�Ҫ������ʱ���жϵ�ʱ�����˸��߼�����жϣ��ᵼ����ʱʱ��䳤

void taskDelay (uint32_t ms) {
	if (ms < SYS_TICK) ms = SYS_TICK;
	
	uint32_t st = enterCritical();

	// �����������Ż�����ΪtaskSched2DelayӦ�ð���taskSched2Unready
	taskSched2Delay(currentTask, ms);
	
	taskSched2Unready(currentTask);
	
	taskSched();
	
	leaveCritical(st);
	
}

void taskTimeSliceHandler() {
	// �����ƣ�����ֱ��ʹ����list�ڲ���Ԫ�ؽ��е�����û�н��з�װ������ʵ��һ���������������װһ��
	for (listNode* node = taskDelayedList.firstNode; node != &(taskDelayedList.headNode); node = node->next) {
		task_t *task = getListNodeParent(node, task_t, delayNode);
		if (--task->delayTicks == 0) {
			if(task->waitEvent) {
				eventRemoveTask(task, NULL, ERROR_TIMEOUT);
			}
			taskSched2Undelay(task);
			taskSched2Ready(task);
		}
	}
	
	if (--currentTask->slice == 0) {
		if (getListNodeNum(&taskTable[currentTask->priority]) > 1) {
			listRemoveFirst(&taskTable[currentTask->priority]);
			listNodeInsert2Tail(&taskTable[currentTask->priority], &currentTask->linkNode);
		}
		currentTask->slice = TIME_SLICE;
	}
	
	taskSched();
}

void SysTick_Handler() {
	taskTimeSliceHandler();
}

