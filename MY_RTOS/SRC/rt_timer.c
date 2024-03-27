#include "rt_timer.h"
#include "semaphore.h"
#include "lock.h"

// ͬһ����ʱ�����ֻ��ͬһ�������ڵ��ã�

static listHead timerStrictList;	// ��ʱ��Ƭ�ж��ڴ���Ķ�ʱ���б�
static listHead timerLooseList;	// �ڶ�ʱ�������д���Ķ�ʱ���б�
static sem_t semForTimerProtect;
static sem_t semForTimerNotify;	// ����֪ͨ��ʱ������ʼִ��,���¼���־��Ҳ��

static task_t timerTask;
static taskStack_t timerTaskStack[TIMER_TASK_STACK_SIZE];

// ���ߺ��������ڱ�����ʱ���б�ִ�е���ʱ����
static void timerCallFuncList(listHead* timerList) {
	for (listNode* node = timerList->firstNode; node != &timerList->headNode; node = node->next) {
		timer_t* timer = getListNodeParent(node, timer_t, linkNode);
		if (timer->rtDelayTicks > 0) {
			if (--timer->rtDelayTicks == 0) {
				timer->state = TIMER_STATE_RUNNING;
				timer->timerFunc(timer->arg);
				timer->state = TIMER_STATE_STARTED;
				
				if (timer->durationTicks != 0) {
					timer->rtDelayTicks = timer->durationTicks;
				} else {
					listRemove(timerList, &timer->linkNode);
					timer->state = TIMER_STATE_STOPPED;
				}
			}
		}
	}
}

// ��ʱ��������
static void timerTaskFunc (void* param) {
	while (1) {
		semWait(&semForTimerNotify, 0);
		// ���ź�����������ΪtimerCallFuncList�漰����timerLooseList�ķ���
		semWait(&semForTimerProtect, 0);
		
		timerCallFuncList(&timerLooseList);
		
		semPost(&semForTimerProtect);
	}
	
}

// ����ʱ��Ƭ�жϵ��ã�ÿһ��tick����һ���źţ�����timerTaskFunc��ʼִ��
// ͬʱ�ᴦ��׼��ʱ���б�
// �����ǹ����жϵ��ã�����Ҫ���ٽ���������
void timerFuncPost(void) {
	timerCallFuncList(&timerStrictList);
	
	semPost(&semForTimerNotify);
}

// ������ʱ������
void timerFuncInit(void) {
	listHeadInit(&timerStrictList);
	listHeadInit(&timerLooseList);
	semInit(&semForTimerProtect, 1, 1);
	semInit(&semForTimerNotify, 0, 0);
	
#if TIMER_TASK_PRIORITY >= (RTOS_PRIORITY_COUNT - 1)
	#error "The priority of timerTask must be greater then (RTOS_PRIORITY_COUNT - 1)"
#endif	
	
	taskInit(&timerTask, timerTaskFunc, NULL, &timerTaskStack[TIMER_TASK_STACK_SIZE], TIMER_TASK_PRIORITY);
}


// ��ʱ��Ҫ��������ʹ�ã���ò�Ҫ�����ȫ�ֱ���
// ��ʼ��һ����ʱ��
void timerInit(timer_t* timer, uint32_t originalDelayTicks, uint32_t durationTicks, void (*timerFunc)(void* arg), void* arg, uint32_t config) {
	listNodeInit(&timer->linkNode);
	timer->originalDelayTicks = originalDelayTicks;
	timer->durationTicks = durationTicks;
	timer->timerFunc = timerFunc;
	timer->arg = arg;
	timer->config = config;
	
	if (originalDelayTicks == 0) {
		timer->rtDelayTicks = durationTicks;
	} else {
		timer->rtDelayTicks = timer->originalDelayTicks;
	}
	
	timer->state = TIMER_STATE_CREATED;
}

// ��ʱ�����ֻ��һ�������ڵ���
// �����ȫ�ֱ����ı���û������
// �ö�ʱ����ʼ��ʱ
void timerStart(timer_t* timer) {
	if (timer->state == TIMER_STATE_CREATED || timer->state == TIMER_STATE_STOPPED) {
		if (timer->config & TIMER_CONFIG_TYPE_STRICT) {
			// �жϴ�����Ҳ�����timerStrictList�����ʹ���ٽ���
			uint32_t st = enterCritical();
			
			listNodeInsert2Tail(&timerStrictList, &timer->linkNode);
			timer->state = TIMER_STATE_STARTED;
			timer->rtDelayTicks = timer->originalDelayTicks ? timer->originalDelayTicks : timer->durationTicks;
			
			leaveCritical(st);
		} else {
			// timerLooseListֻ�ᱻtimerTask���ʣ����ź����Ϳ���
			semWait(&semForTimerProtect, 0);
			listNodeInsert2Tail(&timerLooseList, &timer->linkNode);
			timer->state = TIMER_STATE_STARTED;
			timer->rtDelayTicks = timer->originalDelayTicks ? timer->originalDelayTicks : timer->durationTicks;
			semPost(&semForTimerProtect);
		}
	}
	
}

// �ö�ʱ����ͣ
// ��running��ʱ����ʵ��ͣ���ˣ���ΪҪô�ڶ�ʱ���ж���running��Ҫôrunning��ʱ�������
void timerStop(timer_t* timer) {
	if (timer->state == TIMER_STATE_RUNNING || timer->state == TIMER_STATE_STARTED) {
		if (timer->config & TIMER_CONFIG_TYPE_STRICT) {
			uint32_t st = enterCritical();
			listRemove(&timerStrictList, &timer->linkNode);
			timer->state = TIMER_STATE_STOPPED;
			leaveCritical(st);
		} else {
			semWait(&semForTimerProtect, 0);
			listRemove(&timerLooseList, &timer->linkNode);
			timer->state = TIMER_STATE_STOPPED;
			semPost(&semForTimerProtect);
		}
	}
	
}

// �ָ���ʱ������
void timerResume(timer_t* timer) {
	if (timer->state == TIMER_STATE_STOPPED) {
		
		if (timer->config & TIMER_CONFIG_TYPE_STRICT) {
			// �жϴ�����Ҳ�����timerStrictList�����ʹ���ٽ���
			uint32_t st = enterCritical();
			listNodeInsert2Tail(&timerStrictList, &timer->linkNode);
			timer->state = TIMER_STATE_STARTED;
			leaveCritical(st);
		} else {
			// timerLooseListֻ�ᱻtimerTask���ʣ����ź����Ϳ���
			semWait(&semForTimerProtect, 0);
			listNodeInsert2Tail(&timerLooseList, &timer->linkNode);
			timer->state = TIMER_STATE_STARTED;
			semPost(&semForTimerProtect);
		}
	}
}

// ��������ı���û������
/**
void timerDestory(timer_t* timer) {
	timerStop(timer);
	timer->state = TIMER_STATE_DESTORYED;
}
**/

void timerDestory(timer_t* timer) {
	if (timer->state == TIMER_STATE_RUNNING || timer->state == TIMER_STATE_STARTED) {
		if (timer->config & TIMER_CONFIG_TYPE_STRICT) {
			uint32_t st = enterCritical();
			listRemove(&timerStrictList, &timer->linkNode);
			timer->state = TIMER_STATE_DESTORYED;
			leaveCritical(st);
		} else {
			semWait(&semForTimerProtect, 0);
			listRemove(&timerLooseList, &timer->linkNode);
			timer->state = TIMER_STATE_DESTORYED;
			semPost(&semForTimerProtect);
		}
	}
}

timerInfo_t timerGetInfo(timer_t* timer) {
	timerInfo_t info;
	uint32_t st = enterCritical();
	
	info.originalDelayTicks = timer->originalDelayTicks;
	info.durationDelayTicks = timer->durationTicks;
	info.timerFunc = timer->timerFunc;
	info.state = timer->state;
	info.arg = timer->arg;
	
	leaveCritical(st);
	return info;
}

// ����Ч�ʵİ汾
/**
void timerGetInfo(timer_t* timer, timerInfo_t* info) {
	uint32_t st = enterCritical();
	
	info->originalDelayTicks = timer->originalDelayTicks;
	info->durationDelayTicks = timer->durationTicks;
	info->timerFunc = timer->timerFunc;
	info->state = timer->state;
	info->arg = timer->arg;
	
	leaveCritical(st);
}
**/
