#include "RTOS.h"
#include "misc.h"
#include "LCD.h"
#include "stdio.h"
#include "uart.h"
#include "exevent.h"
#include "exio.h"
#include "cli.h"
#include "string.h"
#include "monitor.h"
#include "intertemperature.h"
#include "light.h"
#include "rng.h"

taskStack_t task1Env[TASK_STACK_SIZE];
taskStack_t task2Env[TASK_STACK_SIZE];
taskStack_t task3Env[TASK_STACK_SIZE];
taskStack_t task4Env[TASK_STACK_SIZE];

task_t ttask1;
task_t ttask2;
task_t ttask3;
task_t ttask4;

int task1Flag;
void task1Entry (void* param) {
	while(1) {
		// char msg[] = "TEXT\r\n";
		// uartWrite(msg, strlen(msg));
		// printf("≤‚ ‘≤‚ ‘≤‚ ‘\r\n");
		int random = RNGGetRandomRange(50, 1000);
		taskDelay(random);
	}
}



int task2Flag;
void task2Entry (void* param) {
	uint8_t x = 0;
	LCD_Init();           //≥ı ºªØLCD FSMCΩ”ø⁄
	while(1) {
		switch(x)
		{
			case 0:LCD_Clear(WHITE);break;
			case 1:LCD_Clear(BLACK);break;
			case 2:LCD_Clear(BLUE);break;
			case 3:LCD_Clear(RED);break;
			case 4:LCD_Clear(MAGENTA);break;
			case 5:LCD_Clear(GREEN);break;
			case 6:LCD_Clear(CYAN);break; 
			case 7:LCD_Clear(YELLOW);break;
			case 8:LCD_Clear(BRRED);break;
			case 9:LCD_Clear(GRAY);break;
			case 10:LCD_Clear(LGRAY);break;
			case 11:LCD_Clear(BROWN);break;
		}
		POINT_COLOR = RED;
		LCD_ShowString(30,40,210,24,24,"SoC: STM32F407ZGT6");	
		LCD_ShowString(30,70,200,16,16,"CPU Core: ARM Cortex-M4");
		LCD_ShowString(30,90,200,16,16,"FPU: ENABLED");
      	
		LCD_ShowString(30,130,200,12,12,"2024/4/19");	      					 
		x++;
		if(x == 12) x=0;
		LED1 = !LED1;
		taskDelay(1000);
	}
}

int task3Flag;
void task3Entry (void* param) {
	while(1) {
		task3Flag = 0;
		taskDelay(10);
		task3Flag = 1;
		taskDelay(10);
	}
}

int task4Flag;
void task4Entry (void* param) {
	while(1) {
		for (int i = 0; i < 0x3f3f3f; ++i) {
			;
		}
		int random = RNGGetRandomRange(500, 1000);
		taskDelay(random);
	}
}

void appInit() {
	uartInit();
	ExIOInit();
	CLIInit();
	exEventInit();
	monitorInit();
	interTemperatureInit();
	lightInit();
	RNGInit();
	taskInit(&ttask1, task1Entry, (void*)0x1145, task1Env, 0, sizeof(task1Env), "TASK1");
	taskInit(&ttask2, task2Entry, (void*)0x1919, task2Env, 1, sizeof(task2Env), "TASK2");
	taskInit(&ttask3, task3Entry, (void*)0x1919, task3Env, 1, sizeof(task3Env), "TASK3");
	taskInit(&ttask4, task4Entry, (void*)0x1919, task4Env, 2, sizeof(task4Env), "TASK4");
}

