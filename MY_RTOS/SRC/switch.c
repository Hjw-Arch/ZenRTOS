#include "RTOS.h"
#include "ARMCM3.h"

task_t* currentTask;
task_t* nextTask;
task_t* taskTable[2];

__asm void PendSV_Handler (void) {
	IMPORT currentTask
	IMPORT nextTask
	
	MRS R0, PSP
//	CBZ R0, noSave 			// 检测标志位，是第一个任务就跳过保存阶段
	
	STMDB R0!, {R4-R11}     // 手动保存R4-R11到当前任务的栈空间，其它的寄存器已经被硬件自动保存
	
	LDR R1, =currentTask    
	LDR R1, [R1]
	STR R0, [R1]			// 将更新后的栈指针保存到当前任务的栈指针中
	
noSave
	LDR R0, =currentTask
	
	LDR R1, =nextTask
	LDR R1, [R1]
	STR R1, [R0]			// 更新currenttask指向nexttask
	
	LDR R0, [R1]
	LDMIA R0!, {R4-R11}     // 从nexttask中手动恢复R4-R11
	
	MSR PSP, R0				// 更新PSP，主要是跳过R4-R11，指向保存R0的地址，以便退出异常时硬件自动恢复寄存器
	ORR LR, LR, #0x04		// 指定LR，即指定退出异常后使用PSP指针而不是MSP
	BX LR
	NOP
}

// 运行第一个任务，将PSP设置为0，作为标志位
// 因为运行第一个任务时候 currenttask是不存在的，因此无法将此时的运行状态保存（也可以，没必要）
// 因此设置一个标志位，在运行第一个任务的时候无需保存之前的运行状态
// 当然，也可以在main函数中先分配一个无意义的currenttask结构，这样就可以将`CBZ R0, noSave`去掉
void runFirstTask() {
	__set_PSP(0);
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI; // 设置PendSVC的优先级
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // 使能PendSVC，触发PendSVC异常
}

void taskSwitch() {
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // 使能PendSVC，触发PendSVC异常
}



