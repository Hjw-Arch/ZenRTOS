#include "RTOS.h"
#include "ARMCM3.h"

task_t* currentTask;
task_t* nextTask;
task_t* taskTable[2];

__asm void PendSV_Handler (void) {
	IMPORT currentTask
	IMPORT nextTask
	
	MRS R0, PSP
	CBZ R0, noSave
	
	STMDB R0!, {R4-R11}
	
	LDR R1, =currentTask
	LDR R1, [R1]
	STR R0, [R1]
	
noSave
	LDR R0, =currentTask
	
	LDR R1, =nextTask
	LDR R1, [R1]
	STR R1, [R0]
	
	LDR R0, [R1]
	LDMIA R0!, {R4-R11}
	
	MSR PSP, R0
	ORR LR, LR, #0x04
	BX LR
}

void runFirstTask() {
	__set_PSP(0);
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI; // 设置PendSVC的优先级
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // 使能PendSVC，触发PendSVC异常
	
	
}

void taskSwitch() {
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // 使能PendSVC，触发PendSVC异常
}



