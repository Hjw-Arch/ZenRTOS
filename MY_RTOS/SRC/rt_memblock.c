#include "rt_memblock.h"
#include "rtLib.h"
#include "lock.h"


// 初始化
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

// 获取一个存储块，如果有存储块剩余就将其从存储块队列中拿走
// 如果没有存储块，则让任务在此等待
// 这个接口，到底用void*好还是uint8_t比较好，有待决定
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
	
	// 如果等到了存储块，存入task->eventMsg中
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


// 归还一个存储块，如果有任务等待在此，则将此存储块直接交给其中一个任务，将此任务唤醒
// 如果唤醒的任务优先级大于当前任务，则进行任务切换
// 如果没有，则将其加入存储块队列
void memBlockPost(memBlock_t* memblock, void* mem) {
	uint32_t st = enterCritical();

	// TODO：
	// 优化方案：判断是否有任务等待，如果有则唤醒一个，这样的逻辑可以直接用eventWakeUp来做
	// eventWakeUp会唤醒一个最高优先级的任务，如果没有任务等待在此会返回NULL，因此可以判断是否有任务在此等待
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

// 此函数是否需要返回值是值得考量的！
// eventRemoveAllTask本身是原子操作，那么本函数加锁还有没有意义？
// 这是个值得考量的问题
// 如果不加锁，有可能会导致没必要的调度tasksched函数，概率不大，但不会对共享变量造成什么影响
// 加锁的代价也不大
// 先放在这
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

// 此函数效率高于上一函数，但不符合习惯
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

