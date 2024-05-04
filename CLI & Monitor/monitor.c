#include "monitor.h"
#include "RTOS.h"
#include "exio.h"
#include "intertemperature.h"
#include "light.h"
#include "uart.h"
#include <string.h>
#include <stdio.h>

#define CURSOR_SAVE				"\033[s"	// 保存光标位置
#define CURSOR_RECOVER			"\033[u"	// 恢复光标位置

#ifdef MONITOR_FUNCTION

task_t monitorTask;
static taskStack_t monitorTaskEnv[MONITOR_TASK_ENV_SIZE];

// 前景色
#define FOREGROUND_NONE                 "\033[0m"			// 默认颜色
#define FOREGROUND_BLACK                "\033[0;30m"
#define FOREGROUND_L_BLACK              "\033[1;30m"
#define FOREGROUND_RED                  "\033[0;31m"
#define FOREGROUND_L_RED                "\033[1;31m"
#define FOREGROUND_GREEN                "\033[0;32m"
#define FOREGROUND_L_GREEN              "\033[1;32m"
#define FOREGROUND_BROWN                "\033[0;33m"
#define FOREGROUND_YELLOW               "\033[1;33m"
#define FOREGROUND_BLUE                 "\033[0;34m"
#define FOREGROUND_L_BLUE               "\033[1;34m"
#define FOREGROUND_PURPLE               "\033[0;35m"
#define FOREGROUND_L_PURPLE             "\033[1;35m"
#define FOREGROUND_CYAN                 "\033[0;36m"
#define FOREGROUND_L_CYAN               "\033[1;36m"
#define FOREGROUND_GRAY                 "\033[0;37m"
#define FOREGROUND_WHITE                "\033[1;37m"


// 工具函数，将浮点数转为字符串，方便打印输出
static void floatToString(float num, char *str, int n) {
    int intPart = (int)num;  // 整数部分
    float floatPart = num - intPart;  // 小数部分

    // 处理整数部分
    int index = 0;
    if (intPart == 0) {
        str[index++] = '0';
    }
    else {
        while (intPart > 0) {
            str[index++] = '0' + (intPart % 10);
            intPart /= 10;
        }
    }
    str[index] = '\0'; // 添加字符串结束符

    // 反转整数部分
    int i = 0, j = index - 1;
    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }

    // 添加小数点
    str[index++] = '.';

    // 处理小数部分
    intPart = (int)floatPart;
	
	if (floatPart == 0) {
		str[--index] = '\0';
		return;
	}
	
    while (index - 1 < n && floatPart > 0) {
        floatPart *= 10;
        intPart = (int)floatPart;
        str[index++] = '0' + intPart;
        floatPart -= intPart;
    }
    str[index] = '\0'; // 添加字符串结束符
}


static void showMonitor() {
	printf("\033[1;34m  ____    _____           __  __    ___    _   _   ___   _____    ___    ____  \r\n");
	printf("\033[1;34m |  _ \\  |_   _|         |  \\/  |  / _ \\  | \\ | | |_ _| |_   _|  / _ \\  |  _ \\ \r\n");
	printf("\033[1;34m | |_) |   | |    _____  | |\\/| | | | | | |  \\| |  | |    | |   | | | | | |_) |\r\n");
	printf("\033[1;34m |  _ <    | |   |_____| | |  | | | |_| | | |\\  |  | |    | |   | |_| | |  _ < \r\n");
	printf("\033[1;34m |_| \\_\\   |_|           |_|  |_|  \\___/  |_| \\_| |___|   |_|    \\___/  |_| \\_\\ \r\n");
	printf("=============================================================================================\r\n\033[0m");
}


static void monitorGetCpuTemp() {
	banPrint;
	int temperature = interTemperGetTemperature();
	allowPrint;
		
	if (temperature < 5000) {
		printf("CPU温度: \033[1;32m%02d.%-2d°C\r\n\033[0m", temperature / 100, (temperature % 100 >= 10 ? temperature % 100 : ((temperature % 100) * 10)));
	} else if (temperature > 5000) {
			printf("CPU温度: \033[1;31m%02d.%-2d°C\r\n\033[0m", temperature / 100, temperature % 100);
	}
}


static void monitorGetCpuUsage() {
	printf("CPU利用率: ");

#if LOW_POWER_MODE == 1
	if (LowPowerMode) {
		printf("\033[1;31mUnavailable\033[0m     ");
		return;
	}
#endif
	
#if FUNCTION_CPUUSAGE_ENABLE == 0
		printf("\033[1;31mEnabled\033[0m     ");
		return;
#endif
	
	float usage = cpuGetUsage();
	if (usage >= 60) {
		printf("\033[1;31m");
	} else if (usage < 60 && usage >= 20) {
		printf("\033[1;34m");
	} else {
		printf("\033[1;32m");
	}
	
	char tool[5];
	floatToString(usage, tool, 3);
	
	printf("%s%%\033[0m     ", tool);
}


void monitorGetTaskInfo() {
	printf("\r\n");
	printf("NAME");			// 4
	printf("\t\t");			// 16
	printf("STATE");		// 21
	printf("\t\t");			// 32
	printf("STACKSIZE");	// 41
	printf("\t");			// 48
	printf("RTUSEDSIZE");	// 58
	printf("\t  ");			// 64 66
	printf("MAXUSEDSIZE");	// 77
	printf("\t\t");			// 88
	printf("\b\b\bPRIORITY");	// 85 93
	printf("\r\n\r\n");
	
	
	for(int i = 0; i < allTaskTableIndex; ++i) {
		if (allTask[i] != NULL) {
			taskInfo_t info = taskGetInfo(allTask[i]);
			
			printf("%s\t", info.taskName);	// taskname
			if (strlen(info.taskName) < 8) {
				printf("\t");
			}
			
			if (allTask[i] == currentTask) {
				printf(FOREGROUND_L_RED "RUNNING \t" FOREGROUND_NONE);
			} else if ((info.state & 0xff << 16)) {			// taskstate
				printf(FOREGROUND_YELLOW "BLOCKED \t" FOREGROUND_NONE);			
			} else if (info.state & TASK_STATUS_READY) {
				printf(FOREGROUND_L_GREEN "READY   \t" FOREGROUND_NONE);
			} else if (info.state & TASK_STATUS_DELAY) {
				printf(FOREGROUND_L_CYAN "DELAYING\t" FOREGROUND_NONE);
			} else if (info.state & TASK_STATUS_SUSPEND) {
				printf(FOREGROUND_L_PURPLE "SUSPEND \t" FOREGROUND_NONE);
			} else if (info.state & TASK_STATUS_DESTORYED) {
				printf("DESTORYED\033[0m\r\n");
				continue;
			} else {
				printf(FOREGROUND_RED "UNKNOWN \t" FOREGROUND_NONE);
			}
			
			printf("%-8d\t", info.stackSize);	// taskstackszie
			
			char tool[5];
			
			floatToString(100.0 - info.stackRtFreeSize*100.0 / info.stackSize, tool, 3);
			if ((info.stackRtFreeSize*100.0 / info.stackSize) < 40) {
				printf("\033[1;31m");
			} else if ((info.stackRtFreeSize*100.0 / info.stackSize) > 90) {
				printf(FOREGROUND_L_CYAN);
			} else {
				printf(FOREGROUND_L_GREEN);
			}
			
			
			printf("%4s%% | %d \t  ", tool, info.stackSize - info.stackRtFreeSize); 	// rtsize
			
			if ((info.stackMinFreeSize*100.0 / info.stackSize) < 40) {
				printf("\033[1;31m");
			} else if ((info.stackMinFreeSize*100.0 / info.stackSize) > 90) {
				printf(FOREGROUND_L_CYAN);
			} else {
				printf(FOREGROUND_L_GREEN);
			}
			
			
			floatToString(100.0 - info.stackMinFreeSize*100.0 / info.stackSize, tool, 3);
			printf("%4s%% | %d \t\t", tool, info.stackSize - info.stackMinFreeSize);	// minsize
			printf("\033[0m");
			
			printf("\b\b\b%d\t\t", info.priority);	// taskpriority
			
			printf("\r\n");
			
		}
	}
	printf("\r\n");
}

static void monitorExIO() {
	
	printf("PF8(BEEP): %s     ", (PFout(8) ? "\033[1;36mHIGH\033[0m" : "\033[0;37mLOW \033[0m"));
	printf("PF9(LED1): %s     ", (PFout(9) ? "\033[1;36mHIGH\033[0m" : "\033[0;37mLOW \033[0m"));
	printf("PF10(LED2): %s   \r\n", (PFout(10) ? "\033[1;36mHIGH\033[0m" : "\033[0;37mLOW \033[0m"));
	
	printf("\r\n");
}

static void showCpuInfo () {
	printf("SoC: \033[1;35mSTM32F407ZGT6     \033[0m");
	printf("CPU Core: \033[1;35mARM Cortex-M4     \033[1;0m");
#if LOW_POWER_MODE == 1
	if (LowPowerMode) printf("Low-power mode: \033[1;32mON     \033[0m");
	else printf("Low-power mode: \033[1;31mOFF     \033[0m");
#else
	printf("Low-power mode: \033[1;31mUnsupported     \033[0m");
#endif
}

static void light() {
	banPrint;
	uint8_t lightValue = lightGetValue();
	allowPrint;
	printf("环境光强度: %-3d\r\n", lightValue);
}

static void monitorTaskEntry(void* param) {
	while(1) {
		printf(CURSOR_SAVE);
		printf(FOREGROUND_YELLOW "CPU information: \r\n\r\n" FOREGROUND_NONE);
		showCpuInfo();
		monitorGetCpuUsage();
		monitorGetCpuTemp();
		printf(FOREGROUND_YELLOW "\r\nTask information: \r\n" FOREGROUND_NONE);
		monitorGetTaskInfo();
		printf(FOREGROUND_YELLOW "External IO: \r\n\r\n" FOREGROUND_NONE);
		monitorExIO();
		light();
		printf("\r\n\033[0;33mPress 'd' or 't' to exit\r\n\r\n\033[0m");
		banPrint;
		taskDelay(1000);
		allowPrint;
		printf(CURSOR_RECOVER);
	}
}

void monitorInit(void) {
	taskInit(&monitorTask, monitorTaskEntry, NULL, monitorTaskEnv, MONITOR_TASK_PRIORITY, sizeof(monitorTaskEnv), "MONITOR");
	taskSuspend(&monitorTask);
}

static void monitorOff(void) {
	lockSched();
	taskSched2Undelay(&monitorTask);
	taskSuspend(&monitorTask);
//	unblockedPrint;
	allowPrint;
	unlockSched();
}

void monitorOn(void) {
//	blockedPrint;
	printf("\033c");
	showMonitor();
	printf(CURSOR_SAVE);
	taskWakeUp(&monitorTask);
	char ch;
re:	uartRead(&ch, 1);
	while (monitorTask.delayTicks <= 1) {
		taskDelay(50);
	}
	if (ch == 'd' || ch == 'T' || ch == 't' || ch == 'D') {
		monitorOff();
	} else {
		goto re;
	}
}



#endif
