#ifndef RT_TASK_H
#define RT_TASK_H

#include <stdint.h>
#include "rtConfig.h"
#include "rtLib.h"

// Ŀǰ�����������Ĳ����ã�һ������ֻ��ͬʱ����һ��״̬������̫������
#define TASK_STATUS_READY		(1 << 0)		// ����״̬������̬������̬
#define TASK_STATUS_DELAY		(1 << 1)		// ����״̬����ʱ̬
#define TASK_STATUS_SUSPEND		(1 << 2)		// ����״̬������̬
#define TASK_STATUS_DESTORYED	(1 << 3)		// ����״̬��ɾ��̬

// ������ԸĽ�
#define TASK_STATUS_WAIT_MASK	(0xff << 16)	// �¼��ȴ����룬��������¼����ƿ�ı�־λ

#define TASK_STATUS_WAIT_SEMAPHORE		(1 << 16)	// ��ʾ�����������ź�����
#define TASK_STATUS_WAIT_MAILBOX		(1 << 17)	// ��ʾ����������������
#define TASK_STATUS_WAIT_MEMBLOCK		(1 << 18)	// ��ʾ���������ڴ洢����
#define TASK_STATUS_WAIT_EFLAGGROUP		(1 << 19)	// ��ʾ�����������¼���־����

// ���������ջ������Ϊuint32
typedef uint32_t taskStack_t;

struct _eventCtrlBlock_t;		// ǰ������

// ��������ṹ
typedef struct _t_Task {
	taskStack_t *stack;  // �����ջָ��
	
	uint32_t slice; // ʱ��Ƭ
	
	uint32_t delayTicks; // ������ʱ���������ڵ�����ʱ����ʱÿSysTick�жϼ�һ
	
	listNode linkNode;
	
	listNode delayNode; // ��ʱ���н��
	
	uint32_t state; // �����ʱ��״̬
	
	uint32_t priority; // ��������ȼ�
	
	uint32_t suspendCounter; // ���������
	
	// ����ɾ����أ�����������
	void (*clean) (void* param);
	void* cleanParam;
	uint8_t requestDeleteFlag;		//����ɾ�����
	
	struct _eventCtrlBlock_t* waitEvent;		// �ȴ����¼����ƿ�
	void* eventMsg;		// �¼���Ϣ
	uint32_t eventWaitResult;	// �¼��ĵȴ����
	
	uint32_t waitEventFlagType;		// �ȴ����¼���־������ͣ���λ���Ǹ�λ
	uint32_t waitEventFlags;		// �ȴ����¼���־����������¼���־�����������̫�ã�Ӧ�ø�һ��
}task_t;

// ����״̬��ѯ�ṹ�����ڱ����ѯ������״̬
typedef struct _taskInfo {
	uint32_t state; // �����ʱ��״̬
	uint32_t slice; // ʱ��Ƭ
	uint32_t priority; // ��������ȼ�
	uint32_t suspendCounter; // ���������
}taskInfo_t;

extern task_t _idleTask;
extern task_t* currentTask;
extern task_t* nextTask;
extern task_t* idleTask;
extern listHead taskTable[RTOS_PRIORITY_COUNT];
extern taskStack_t idleTaskEnv[512];

extern listHead taskDelayedList;

extern Bitmap taskPriorityBitmap;


void runFirstTask(void);
void runFirstTask2(void);
void taskSwitch(void);

void taskInit (task_t* task, void (*entry)(void*), void* param, taskStack_t* stack, uint32_t priority);
void taskSched(void);

void idleTaskEntry (void* param);
task_t* getHighestReadyTask(void);
void taskDelayedListInit(void);
void taskSched2Ready(task_t* task);
void taskSched2Unready(task_t* task);

void taskSched2Delay(task_t* task, uint32_t ms);
void taskSched2Undelay(task_t* task);

void taskSuspend(task_t* task);
void taskWakeUp(task_t* task);

void taskSetCleanCallFunc (task_t* task, void (*clean)(void* param), void* param);
void taskForceDelete (task_t* task);
void taskRequestDelete(task_t* task);
uint8_t taskIsRequestedDelete(task_t* task);
void taskDeleteSelf(void);

taskInfo_t taskGetInfo(task_t* task);
// void taskGetInfo(task_t* task, taskInfo_t* taskinfo); // �˰汾�������һ��������Сһ�㣬����С�Ĳ���

#endif


