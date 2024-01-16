#include <RTOS.h>



void triggerPenderSVC (void) {
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack) {
	task->stack = stack;
}

typedef struct _blockType_t {
	unsigned long* stackPtr;
}blockType_t;

blockType_t* blockPtr;

int flag;

void delay(int count) {
	while (--count) ;
}

unsigned long stackBuffer[1024];
blockType_t block;

task_t ttask1;
task_t ttask2;

taskStack_t task1Env[1024];
taskStack_t task2Env[1024];

void task1 (void* param) {
	while(1) {
		
	}
}

void task2 (void* param) {
	while(1) {
		
	}
}

int main(){
	taskInit(&ttask1, task1, (void*)0x1145, &task1Env[1024]);
	taskInit(&ttask2, task2, (void*)0x1919, &task2Env[1024]);
	
	block.stackPtr = &stackBuffer[1024];
	
	blockPtr = &block;
	
	for(;;) {
		flag = 0;
		delay(100);
		flag = 1;
		delay(100);
		
		triggerPenderSVC();
	}
}

