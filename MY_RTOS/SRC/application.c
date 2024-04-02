#include "RTOS.h"

taskStack_t task1Env[TASK_STACK_SIZE];
taskStack_t task2Env[TASK_STACK_SIZE];
taskStack_t task3Env[TASK_STACK_SIZE];
taskStack_t task4Env[TASK_STACK_SIZE];

task_t ttask1;
task_t ttask2;
task_t ttask3;
task_t ttask4;

taskInfo_t taskinfo1, taskinfo2, taskinfo3, taskinfo4;

timer_t timer1, timer2;

uint32_t bit1, bit2;


// ��ʱ��������ÿ�η�תbitλ
void timerFunc(void* arg) {
	uint32_t* ptrBit = (uint32_t*)arg;
	*ptrBit ^= 0x1;
}

float cpuUsage = 0.0f;

// ����
void* messageBuffer[16];
mbox_t mbox;

int task1Flag;
void task1Entry (void* param) {
	uint32_t msg[16];
	mboxInit(&mbox, messageBuffer, 16);
	
	// ��ʼ����ʱ��
	// ��ʱ��1��ϵͳ������2000ms��ʼ���У����ÿ��100ms����һ�ζ�ʱ������
	timerInit(&timer1, 200, 10, timerFunc, &bit1, TIMER_CONFIG_TYPE_STRICT);
	timerStart(&timer1);
	// ��ʱ��2��ϵͳ������4000ms��ʼ���У����ÿ��200ms����һ�ζ�ʱ������
	timerInit(&timer2, 400, 20, timerFunc, &bit2, TIMER_CONFIG_TYPE_LOOSE);
	timerStart(&timer2);
	
	while(1) {
		// ͨ�������䷢����Ϣ
		for (int i = 0; i < 16; ++i) {
			msg[i] = i;
			mboxPost(&mbox, &msg[i], PRIORITY_NORMAL);
			task1Flag = i;
		}
		
		taskDelay(500);
		
		
		for (int i = 0; i < 16; ++i) {
			msg[i] = i;
			mboxPost(&mbox, &msg[i], PRIORITY_HIGH);
			task1Flag = i;
		}
		
		taskDelay(500);
		
		task1Flag = 0;
		taskDelay(10);
		task1Flag = 1;
		taskDelay(10);
		
		taskinfo1 = taskGetInfo(currentTask);
	}
}


int task2Flag;
void task2Entry (void* param) {
	while(1) {
		// ����2 ����������Ϣ
		void* msg;
		
		uint32_t err = mboxWait(&mbox, &msg, 0);
		if (err == NO_ERROR) {
			task2Flag = *(int *)msg;
			taskDelay(10);
		}
		
		task2Flag = 0;
		taskDelay(10);
		task2Flag = 1;
		taskDelay(10);
		
		taskinfo2 = taskGetInfo(&ttask2);
	}
}

int task3Flag;
void task3Entry (void* param) {
	while(1) {
		void* msg;
		
		uint32_t err = mboxWait(&mbox, &msg, 0);
		if (err == NO_ERROR) {
			task3Flag = *(int *)msg;
			taskDelay(10);
		}
		task3Flag = 0;
		taskDelay(10);
		task3Flag = 1;
		taskDelay(10);
		
		taskinfo3 = taskGetInfo(&ttask3);
	}
}

int task4Flag;
void task4Entry (void* param) {
	while(1) {
		void* msg;
		
		uint32_t err = mboxWait(&mbox, &msg, 0);
		if (err == NO_ERROR) {
			task4Flag = *(int *)msg;
			taskDelay(10);
		}
		task4Flag = 0;
		taskDelay(10);
		task4Flag = 1;
		taskDelay(10);
		
		// ����CPUʹ����
		cpuUsage = cpuGetUsage();
		
		// ��ѯ�����ջʹ�����
		taskinfo4 = taskGetInfo(&ttask4);
	}
}

void appInit() {
	taskInit(&ttask1, task1Entry, (void*)0x1145, task1Env, 0, sizeof(task1Env));
	taskInit(&ttask2, task2Entry, (void*)0x1919, task2Env, 1, sizeof(task2Env));
	taskInit(&ttask3, task3Entry, (void*)0x1919, task3Env, 1, sizeof(task3Env));
	taskInit(&ttask4, task4Entry, (void*)0x1919, task4Env, 2, sizeof(task4Env));
}

