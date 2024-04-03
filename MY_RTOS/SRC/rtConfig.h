#ifndef RTCONFIG_H
#define RTCONFIG_H

typedef enum _error {
	NO_ERROR = 0,
	ERROR_TIMEOUT = 1,
	ERROR_RESOURCE_UNAVAILABLE,
	ERROR_DELETED,
	ERROR_MAILBOX_FULL,
	ERROR_MEMBLOCK_FULL,
	ERROR_UNMATCHED_OWNER,
}rt_error;

#define RTOS_PRIORITY_COUNT			32

#define SYS_TICK					10  // ϵͳ���ģ���λ��ms�� ��ÿSYS_TICK ms����һ��ϵͳʱ���ж�

#define TIME_SLICE					10  // ����ʱ��Ƭ����λ��ϵͳ����

#define TASK_STACK_SIZE				1024  // Ĭ�ϵ������ջ��С

#define TIMER_TASK_STACK_SIZE		1024	// ��ʱ������Ķ�ջ��С
#define TIMER_TASK_PRIORITY			1		// ��ʱ����������ȼ���ע�⣬���ܺͿ�����������ȼ�һ�£����������ᰴʱ��Ƭ��������

// ��ʵʱ��ģʽ
#define HIGH_RT_MODE				1		// ������ʵʱ��ģʽ����ʹ�������һЩ����ʱ�������������ڻ���ʱ�����Ȼ��Ѹ����ȼ������п��ܵ��µ����ȼ�����ʱ��ò�������

#define	FUNCTION_SEMAPHORE_ENABLE	1		// �ź�������
#define	FUNCTION_MUTEX_EBABLE		1		// �����ź���
#define	FUNCTION_EFLAGGROUP_EBABLE	1		// �¼���־��
#define	FUNCTION_MBOX_ENABLE		1		// ����
#define	FUNCTION_MBLOCK_ENABLE		1		// �洢��
#define	FUNCTION_SOFTTIMER_ENABLE	1		// ��ʱ��			�˹���Ҫ���ź������뿪��
#define	FUNCTION_CPUUSAGE_ENABLE	1		// CPUʹ����ͳ��	�رմ˹��ܣ���hooks���ܲ���hooksCpuIdleʵ�ֵ͹���ģʽ
#define FUNCTION_HOOKS_ENABLE		0		// ���Ӻ���


#endif
