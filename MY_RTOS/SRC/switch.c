// ��ߵĺ���û����������Ȼ���ܣ����ǲ��淶
#include "RTOS.h"
#include "ARMCM3.h"

#define MEM32(addr)					*(volatile unsigned long*)(addr)
#define MEM8(addr)					*(volatile unsigned char*)(addr)
	
#define NVIC_INT_CTRL				0xe000ed04	// �жϿ��Ƽ�״̬�Ĵ���ICSR
#define NVIC_PENDSVSET				0x10000000  //  PendSVC�ж�ʹ��λ��ʹ��PendSVC�ж�
#define NVIC_SYSPRI2				0xe000ed22	//  PendSVC�����ȼ��Ĵ���
#define NVIC_PENDSV_PRI				0xff  //  PendSV���ȼ�������λ������ȼ�

__asm void PendSV_Handler (void) {
	IMPORT currentTask
	IMPORT nextTask
	
	MRS R0, PSP

//  ʹ��runfirsttak����Ҫ��������ָ�ʹ��runfirsttask2������Ҫ��������ָ��
//	CBZ R0, noSave 			// ����־λ���ǵ�һ���������������׶�,
	
	STMDB R0!, {R4-R11}     // �ֶ�����R4-R11����ǰ�����ջ�ռ䣬�����ļĴ����Ѿ���Ӳ���Զ�����
	
	LDR R1, =currentTask    
	LDR R1, [R1]
	STR R0, [R1]			// �����º��ջָ�뱣�浽��ǰ�����ջָ����
	
noSave
	LDR R0, =currentTask
	
	LDR R1, =nextTask
	LDR R1, [R1]
	STR R1, [R0]			// ����currenttaskָ��nexttask
	
	LDR R0, [R1]
	LDMIA R0!, {R4-R11}     // ��nexttask���ֶ��ָ�R4-R11
	
	MSR PSP, R0				// ����PSP����Ҫ������R4-R11��ָ�򱣴�R0�ĵ�ַ���Ա��˳��쳣ʱӲ���Զ��ָ��Ĵ���
	ORR LR, LR, #0x04		// ָ��LR����ָ���˳��쳣��ʹ��PSPָ�������MSP
	BX LR
	NOP
}

// ���е�һ�����񣬽�PSP����Ϊ0����Ϊ��־λ
// ��Ϊ���е�һ������ʱ�� currenttask�ǲ����ڵģ�����޷�����ʱ������״̬���棨Ҳ���ԣ�û��Ҫ��
// �������һ����־λ�������е�һ�������ʱ�����豣��֮ǰ������״̬
// ��Ȼ��Ҳ������main�������ȷ���һ���������currenttask�ṹ�������Ϳ��Խ�`CBZ R0, noSave`ȥ��
void runFirstTask() {
	__set_PSP(0);
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI; // ����PendSVC�����ȼ�
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // ʹ��PendSVC������PendSVC�쳣
}

void runFirstTask2() {
//	��idletask��ջ�����ã����������л�����������������ǰ��R4-R11��û���ã������������˷ѿռ䡣
//	ֻҪ��psp��currenttaskָ��һ�����õĵ�ַ�Ϳ�����
	__set_PSP((uint32_t)(&idleTaskEnv[TASK_STACK_SIZE - 1 - 16]));
	currentTask = (task_t*)(&idleTaskEnv[TASK_STACK_SIZE - 1 - 16]);		// ��currenttaskָ��idletaskջ�����������л�����ʹ�ã�û���ã������������˷ѿռ䡣
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;			// ����PendSVC�����ȼ�
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; 			// ʹ��PendSVC������PendSVC�쳣
}


void taskSwitch() {
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // ʹ��PendSVC������PendSVC�쳣
}



