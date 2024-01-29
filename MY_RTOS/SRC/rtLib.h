#ifndef RTLIB_H
#define RTLIB_H

#include <stdint.h>

#define NULL		((void*)0)


// 位图
typedef struct {
	uint32_t bitmap;
}Bitmap;

void bitmapInit(Bitmap* bitmap);
void bitmapSet(Bitmap* bitmap, uint32_t pos);
void bitmapClear(Bitmap* bitmap, uint32_t pos);
uint32_t bitmapPosCount(void);
uint32_t bitmapGetFirstSet(Bitmap* bitmap);



// 通用链表

typedef struct _ListNode {
	struct _ListNode* prev;
	struct _ListNode* next;
}listNode;

typedef struct _ListHead {
	listNode* firstNode;
	listNode* lastNode;
	uint32_t nodeCount;
}listHead;

#define getListNodeParent(listnode, parent, name) (parent*)((uint32_t)listnode - (uint32_t)&((parent*)0)->name)


void listNodeInit(listNode* listnode);
void listHeadInit(listHead* listhead);
uint32_t getListNodeNum(listHead* listhead);
listNode* getFirstListNode(listHead* listhead);
listNode* getLastListNode(listHead* listhead);
listNode* getPrevListNode (listNode* listnode);
listNode* getNextListNode (listNode* listnode);
void listClearALL (listHead* listhead);
void listNodeInsert2Head (listHead* listhead, listNode* listnode);
void listNodeInsert2Tail (listHead* listhead, listNode* listnode);
listNode* listRemoveFirst (listHead* listhead);
void listInsert(listHead* listhead, listNode* node, listNode* node2insert);
void listRemove(listHead* listhead, listNode* node2remove);


#endif
