#include "RTOS.h"
#include "ARMCM3.h"


task_t* idleTaskp;

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
	
	task->delayTicks = 0;
}

// 任务调度函数，来决定下一个运行的任务是哪个
void taskSched(void) {
	if (currentTask == idleTaskp) {
		if (taskTable[0]->delayTicks == 0) nextTask = taskTable[0];
		else if (taskTable[1]->delayTicks == 0) nextTask = taskTable[1];
		else return;
	}
	
	if (currentTask == taskTable[0] && taskTable[1]->delayTicks == 0) {
		nextTask = taskTable[1];
	} else if (currentTask == taskTable[1] && taskTable[0]->delayTicks == 0){
		nextTask = taskTable[0];
	} else if (taskTable[0]->delayTicks != 0 && taskTable[1] != 0) {
		nextTask = idleTaskp;
	}
	
	taskSwitch();
}

// 要实现任务延时，需要使用定时器，而且每个任务都配备一个定时器才行，但是硬件只有一个定时器而任务数量很多
// 因此可以利用SysTick这个硬件定时器来实现软定时器
// 因为SysTick周期性触发中断，因此可以以这个周期为最基本的软件定时器的时间单位
// 每触发一次SysTick就将软定时器的值-1即可
// 因此软定时器的定时时间都是SysTick中断的倍数
// 中断处理也需要时间，不能太频繁这样会导致系统在切换任务这个事情上占用太多资源，本末倒置，一般设置为10ms-100ms就可以


// 软定时器延时精度并不准确，要注意使用场合 eg：延时一个单位，在两个SysTick中间开始延时，那么只能延时半个SysTick中断周期。
// 假如在将要触发定时器中断的时候发生了更高级别的中断，会导致延时时间变长

void taskDelay (uint32_t ms) {
	if (ms < TIME_SLICE) ms = TIME_SLICE;
	
	currentTask->delayTicks = (ms + TIME_SLICE / 2) / TIME_SLICE; // 四舍五入算法
	
	taskSched();
}

void taskTimeSliceHandler() {
	for (int i = 0; i < 2; ++i) {
		if (taskTable[i]->delayTicks > 0) {
			taskTable[i]->delayTicks--;
		}
	}
	
	taskSched();
}

// 设置SysTick定时中断的时间（中断周期，任务的时间片）
// 
void setSysTick(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; // 设定预载值
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);  //设定优先级
	SysTick->VAL = 0; //设定定时器值
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; //使用内核定时器；启动定时中断；使能SysTick
}

void SysTick_Handler() {
	taskTimeSliceHandler();
}


int task1Flag;
void task1Entry (void* param) {
	setSysTick(TIME_SLICE);
	while(1) {
		task1Flag = 0;
		taskDelay(20);
		task1Flag = 1;
		taskDelay(20);
	}
}

int task2Flag;
void task2Entry (void* param) {
	while(1) {
		task2Flag = 0;
		taskDelay(20);
		task2Flag = 1;
		taskDelay(20);
	}
}

task_t ttask1;
task_t ttask2;
task_t idleTask;

taskStack_t task1Env[1024];
taskStack_t task2Env[1024];
taskStack_t idleTaskEnv[512];


void idleTaskEntry (void* param) {
	while(1) ;
}

int main(){
	taskInit(&ttask1, task1Entry, (void*)0x1145, &task1Env[1024]);
	taskInit(&ttask2, task2Entry, (void*)0x1919, &task2Env[1024]);
	
	// idletask
	taskInit(&idleTask, idleTaskEntry, (void*)0, &idleTaskEnv[512]);
	
	taskTable[0] = &ttask1;
	taskTable[1] = &ttask2;
	
	nextTask = &ttask1;
	idleTaskp = &idleTask;
	
//	runFirstTask(); // 运行之后不会返回，下方的return 0其实没什么作用
	
//	将idletask的栈拿来用，将在任务切换中来保存任务运行前的R4-R11（没有用），这样不会浪费空间。
//	只要将psp和currenttask指向一段无用的地址就可以了
	__set_PSP((uint32_t)(&idleTaskEnv[256]));
	currentTask = (task_t*)(&idleTaskEnv[256]);		// 将currenttask指向idletask栈区，在任务切换中来使用（没有用），这样不会浪费空间。
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;			// 设置PendSVC的优先级
	
	taskSwitch();
	
	return 0;
}

