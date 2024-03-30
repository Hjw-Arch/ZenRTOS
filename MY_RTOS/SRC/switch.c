// 这边的函数没有声明，虽然能跑，但是不规范
#include "RTOS.h"
#include "ARMCM3.h"

#define MEM32(addr)					*(volatile unsigned long*)(addr)
#define MEM8(addr)					*(volatile unsigned char*)(addr)
	
#define NVIC_INT_CTRL				0xe000ed04	// 中断控制及状态寄存器ICSR
#define NVIC_PENDSVSET				0x10000000  //  PendSVC中断使能位，使能PendSVC中断
#define NVIC_SYSPRI2				0xe000ed22	//  PendSVC的优先级寄存器
#define NVIC_PENDSV_PRI				0xff  //  PendSV优先级，设置位最低优先级

__asm void PendSV_Handler (void) {
	IMPORT currentTask
	IMPORT nextTask
	
	MRS R0, PSP

//  使用runfirsttak，需要下面这条指令，使用runfirsttask2，不需要下面这条指令
//	CBZ R0, noSave 			// 检测标志位，是第一个任务就跳过保存阶段,
	
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

void runFirstTask2() {
//	将idletask的栈拿来用，将在任务切换中来保存任务运行前的R4-R11（没有用），这样不会浪费空间。
//	只要将psp和currenttask指向一段无用的地址就可以了
	__set_PSP((uint32_t)(&idleTaskEnv[TASK_STACK_SIZE - 1 - 16]));
	currentTask = (task_t*)(&idleTaskEnv[TASK_STACK_SIZE - 1 - 16]);		// 将currenttask指向idletask栈区，在任务切换中来使用（没有用），这样不会浪费空间。
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;			// 设置PendSVC的优先级
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; 			// 使能PendSVC，触发PendSVC异常
}


void taskSwitch() {
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // 使能PendSVC，触发PendSVC异常
}



