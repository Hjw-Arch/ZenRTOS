#include "rt_memblock.h"
#include "rtLib.h"
#include "lock.h"


// ��ʼ��
uint32_t memBlockInit(memBlock_t* memBlock, void* memStart, uint32_t blockSize, uint32_t blockNum){
	uint8_t* memBlockStart = (uint8_t*)memStart;
	uint8_t* memBlockEnd = memBlockStart + blockSize * blockNum;
	
	if (blockSize < sizeof(listNode)) {
		return 1;
	}
	
	eventInit(&memBlock->event, EVENT_TYPE_MEMBLOCK);
	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->maxCount = blockNum;
	listHeadInit(&memBlock->blockList);
	
	while (memBlockStart < memBlockEnd) {
		listNodeInit((listNode*)memBlockStart);
		listNodeInsert2Tail(&memBlock->blockList, (listNode*)memBlockStart);
		
		memBlockStart += blockSize;
	}
	
	return 0;
}

// ��ȡһ���洢�飬����д洢��ʣ��ͽ���Ӵ洢�����������
// ���û�д洢�飬���������ڴ˵ȴ�
// ����ӿڣ�������void*�û���uint8_t�ȽϺã��д�����
uint32_t memBlockWait(memBlock_t* memblock, void* mem, uint32_t waitTime) {
	uint32_t st = enterCritical();
	
	if (getListNodeNum(&memblock->blockList) > 0) {
		*(void**)mem = listRemoveFirst(&memblock->blockList);
		leaveCritical(st);
		return NO_ERROR;
	}
	
	eventWait(&memblock->event, currentTask, NULL, TASK_STATUS_WAIT_MEMBLOCK, waitTime);
	
	taskSched();
	leaveCritical(st);
	
	// ����ȵ��˴洢�飬����task->eventMsg��
	*(void**)mem = currentTask->eventMsg;
	return currentTask->eventWaitResult;
}

uint32_t memBlockGetWithNoWait(memBlock_t* memblock, void* mem) {
	uint32_t st = enterCritical();
	
	
	if (getListNodeNum(&memblock->blockList) > 0) {
		*(void**)mem = listRemoveFirst(&memblock->blockList);
		leaveCritical(st);
		return NO_ERROR;
	}
	
	
	leaveCritical(st);
	return ERROR_RESOURCE_UNAVAILABLE;
}


// �黹һ���洢�飬���������ȴ��ڴˣ��򽫴˴洢��ֱ�ӽ�������һ�����񣬽���������
// ������ѵ��������ȼ����ڵ�ǰ��������������л�
// ���û�У��������洢�����
void memBlockPost(memBlock_t* memblock, void* mem) {
	uint32_t st = enterCritical();

	// TODO��
	// �Ż��������ж��Ƿ�������ȴ������������һ�����������߼�����ֱ����eventWakeUp����
	// eventWakeUp�ỽ��һ��������ȼ����������û������ȴ��ڴ˻᷵��NULL����˿����ж��Ƿ��������ڴ˵ȴ�
	if(eventGetWaitNum(&memblock->event) > 0) {
		task_t* task = eventWakeUp(&memblock->event, mem, NO_ERROR);
		if (task->priority < currentTask->priority) {
			taskSched();
		}
		leaveCritical(st);
		return;
	}
	
	listNodeInsert2Tail(&memblock->blockList, (listNode*)mem);
	
	leaveCritical(st);
}

// �˺����Ƿ���Ҫ����ֵ��ֵ�ÿ����ģ�
// eventRemoveAllTask������ԭ�Ӳ�������ô��������������û�����壿
// ���Ǹ�ֵ�ÿ���������
// ������������п��ܻᵼ��û��Ҫ�ĵ���tasksched���������ʲ��󣬵�����Թ���������ʲôӰ��
// �����Ĵ���Ҳ����
// �ȷ�����
uint32_t memBlockDestory(memBlock_t* memblock) {
	uint32_t st = enterCritical();
	
	uint32_t count = eventRemoveAllTask(&memblock->event, NULL, ERROR_DELETED);
	
	if (count > 0) {
		taskSched();
	}
	
	leaveCritical(st);
	
	return count;
}

memBlockInfo_t memBlockGetInfo(memBlock_t* memblock) {
	memBlockInfo_t info;
	
	uint32_t st = enterCritical();
	
	info.blockSize = memblock->blockSize;
	info.maxCount = memblock->maxCount;
	info.memBlockNum = getListNodeNum(&memblock->blockList);
	info.waitTaskNum = eventGetWaitNum(&memblock->event);
	
	leaveCritical(st);
	
	return info;
}

// �˺���Ч�ʸ�����һ��������������ϰ��
/**
void memBlockGetInfo(memBlock_t* memblock, memBlockInfo_t* info) {
	uint32_t st = enterCritical();
	
	info->blockSize = memblock->blockSize;
	info->maxCount = memblock->maxCount;
	info->memBlockNum = getListNodeNum(&memblock->blockList);
	info->waitTaskNum = eventGetWaitNum(&memblock->event);
	
	leaveCritical(st);
}
**/

