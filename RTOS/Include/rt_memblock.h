#ifndef RT_MEMBLOCK_H
#define RT_MEMBLOCK_H

#include "rt_event.h"

// �洢��ṹ
typedef struct _memblock {
	eventCtrlBlock_t event;		// �¼����ƿ飬�����ʱû�пռ�ȥ����ͽ����������ڴ�
	void* memStart;				// �洢��Ŀ�ʼ��ַ
	uint32_t blockSize;			// ÿһ���洢��Ĵ�С
	uint32_t maxCount;			// �洢�������
	listHead blockList;			// ���Ӹ����洢��
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
// void memBlockGetInfo(memBlock_t* memblock, memBlockInfo_t* info);	//�˺���Ч���Ը�����һ������


#endif


