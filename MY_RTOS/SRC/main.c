#include "RTOS.h"
#include "ARMCM3.h"


void triggerPenderSVC (void) {
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

// 任务初始化
void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack) {
	// 进入中断/异常时， 硬件会自动将8个寄存器压栈，顺序是xPSR、PC(R15)、LR(R14)、R12以及R3-R0
	*(--stack) = (unsigned long)(1 << 24); // xPSR中第24位，即T标志位设置为1，否则进入ARM模式，这在CM3上不允许！
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x03;
	*(--stack) = (unsigned long)0x02;
	*(--stack) = (unsigned long)0x01;
	*(--stack) = (unsigned long)param;
	
	// 手动保存R11-R4寄存器
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x09;
	*(--stack) = (unsigned long)0x08;
	*(--stack) = (unsigned long)0x07;
	*(--stack) = (unsigned long)0x06;
	*(--stack) = (unsigned long)0x05;
	*(--stack) = (unsigned long)0x04;
	
		
	task->stack = stack;
}

void taskSched(void) {
	if (currentTask == *taskTable) {
		nextTask = *(taskTable + 1);
	} else {
		nextTask = *taskTable;
	}
	
	taskSwitch();
}

// 设置SysTick定时时间
void setSysTick(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; // 设定预载值
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);  //设定优先级
	SysTick->VAL = 0; //设定定时器值
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; //使用内核定时器；启动定时中断；使能SysTick
}

void SysTick_Handler() {
	taskSched();
}


void delay(int count) {
	while (--count) ;
}

task_t ttask1;
task_t ttask2;

taskStack_t task1Env[1024];
taskStack_t task2Env[1024];

int task1Flag;
void task1Entry (void* param) {
	setSysTick(10);
	while(1) {
		task1Flag = 0;
		delay(100);
		task1Flag = 1;
		delay(100);
	}
}

int task2Flag;
void task2Entry (void* param) {
	while(1) {
		task2Flag = 0;
		delay(100);
		task2Flag = 1;
		delay(100);
	}
}

int main(){
	
	taskInit(&ttask1, task1Entry, (void*)0x1145, &task1Env[1024]);
	taskInit(&ttask2, task2Entry, (void*)0x1919, &task2Env[1024]);
	
	taskTable[0] = &ttask1;
	taskTable[1] = &ttask2;
	
	nextTask = &ttask1;
	
	runFirstTask(); // 运行之后不会返回，下方的return 0其实没什么作用
	
	return 0;
}

