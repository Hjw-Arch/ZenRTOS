#include "rtLib.h"

#define firstNode headNode.next
#define lastNode  headNode.prev

// 初始化结点,指向null
void listNodeInit(listNode* listnode) {
	listnode->prev = NULL;
	listnode->next = NULL;
}

// 初始化链表头
void listHeadInit(listHead* listhead) {
	listhead->firstNode = &(listhead->headNode);
	listhead->lastNode = &(listhead->headNode);
	listhead->nodeCount = 0;
}

uint32_t getListNodeNum(listHead* listhead) {
	return listhead->nodeCount;
}

listNode* getFirstListNode(listHead* listhead) {
	if (listhead->nodeCount == 0){
		return NULL;
	}
	return listhead->firstNode;
}

listNode* getLastListNode(listHead* listhead) {
	if (listhead->nodeCount == 0) {
		return NULL;
	}
	return listhead->lastNode;
}

listNode* getPrevListNode (listHead* listhead, listNode* listnode) {
	if (listnode->prev == &(listhead->headNode)) {
		return NULL;
	}
	else {
		return listnode->prev;
	}
}

listNode* getNextListNode (listHead* listhead, listNode* listnode) {
	if (listnode->next == &(listhead->headNode)) {
		return NULL;
	}
	else {
		return listnode->next;
	}
}

void listClearALL (listHead* listhead) {
	listNode* nextnode;
	
	nextnode = listhead->firstNode;
	
	for (int i = listhead->nodeCount; i > 0; --i) {
		listNode* currentnode = nextnode;
		nextnode = nextnode->next;
		
		currentnode->prev = NULL;
		currentnode->next = NULL;
	}
	
	listhead->nodeCount = 0;
	listhead->firstNode = &(listhead->headNode);
	listhead->lastNode = &(listhead->headNode);
}

void listNodeInsert2Head (listHead* listhead, listNode* listnode) {
	listnode->prev = listhead->firstNode->prev;
	listnode->next = listhead->firstNode;
	
	listhead->firstNode->prev = listnode;
	listhead->firstNode = listnode;
	listhead->nodeCount++;
}


void listNodeInsert2Tail (listHead* listhead, listNode* listnode) {
	listnode->next = listhead->lastNode->next;
	listnode->prev = listhead->lastNode;
	
	listhead->lastNode->next = listnode;
	listhead->lastNode = listnode;
	listhead->nodeCount++;
}

listNode* listRemoveFirst (listHead* listhead) {
	listNode* node = listhead->firstNode;
	if (listhead->nodeCount == 0) {
		return NULL;
	}
	else {
		listhead->firstNode->next->prev = listhead->firstNode->prev;
		listhead->firstNode = listhead->firstNode->next;
		listhead->nodeCount--;
	}
	return node;
}

void listInsert(listHead* listhead, listNode* node, listNode* node2insert) {
	node2insert->prev = node;
	node2insert->next = node->next;
	
	node->next->prev = node2insert;
	node->next = node2insert;
	
	listhead->nodeCount++;
}

void listRemove(listHead* listhead, listNode* node2remove) {
	node2remove->next->prev = node2remove->prev;
	node2remove->prev->next = node2remove->next;
	
	listhead->nodeCount--;
}








