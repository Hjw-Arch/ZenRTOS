#ifndef RTLIB_H
#define RTLIB_H

#include <stdint.h>


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
	listNode headNode;
	uint32_t nodeCount;
}listHead;

#endif
