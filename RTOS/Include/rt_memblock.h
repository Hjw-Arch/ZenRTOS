#ifndef RT_MEMBLOCK_H
#define RT_MEMBLOCK_H

#include "rt_event.h"

// 存储块结构
typedef struct _memblock {
	eventCtrlBlock_t event;		// 事件控制块，如果暂时没有空间去分配就将任务阻塞在此
	void* memStart;				// 存储块的开始地址
	uint32_t blockSize;			// 每一个存储块的大小
	uint32_t maxCount;			// 存储块的数量
	listHead blockList;			// 连接各个存储块
}memBlock_t;

typedef struct _memblockinfo {
	uint32_t memBlockNum;
	uint32_t waitTaskNum;
	uint32_t maxCount;
	uint32_t blockSize;
}memBlockInfo_t;


uint32_t memBlockInit(memBlock_t* memBlock, void* memStart, uint32_t blockSize, uint32_t blockNum);
uint32_t memBlockWait(memBlock_t* memblock, void* mem, uint32_t waitTime);
uint32_t memBlockGetWithNoWait(memBlock_t* memblock, void* mem);
void memBlockPost(memBlock_t* memblock, void* mem);
uint32_t memBlockDestory(memBlock_t* memblock);
memBlockInfo_t memBlockGetInfo(memBlock_t* memblock);
// void memBlockGetInfo(memBlock_t* memblock, memBlockInfo_t* info);	//此函数效率略高于上一个函数


#endif


