#ifndef RTCONFIG_H
#define RTCONFIG_H

typedef enum _error {
	NO_ERROR = 0,
	ERROR_TIMEOUT = 1,
	ERROR_RESOURCE_UNAVAILABLE,
	ERROR_DELETED,
}rt_error;

#define RTOS_PRIORITY_COUNT			32

#define SYS_TICK					10  // ϵͳ���ģ���λ��ms

#define TIME_SLICE					10  // ����ʱ��Ƭ����λ��ϵͳ����

#define TASK_STACK_SIZE				1024  // Ĭ�ϵ������ջ��С

// ��ʵʱ��
#define HIGH_RT_MODE



#endif
