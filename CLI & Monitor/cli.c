#include "RTOS.h"
#include "cli.h"
#include "uart.h"
#include "button.h"
#include "exio.h"
#include "monitor.h"
#include "intertemperature.h"
#include "light.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define KEYBOARD_DIR1		0x1B
#define KEYBOARD_DIR2		0x5B
#define KEYBOARD_LEFT		'D'
#define KEYBOARD_RIGHT		'C'
#define KEYBOARD_UP			'A'
#define KEYBOARD_DOWN		'B'

#define KEYBOARD_DELETE		0x7f
#define KEYBOARD_BS			0x08
#define KEYBOARD_SPACE		' '
#define KEYBOARD_CR			'\r'
#define KEYBOARD_LR			'\n'
#define KEYBOARD_TAB		'\t'

#define CLS				printf("\033[2J")				// 屏幕翻页
#define CLEAR			printf("\033c")					// 彻底清屏
#define MOVEUP(x)		printf("\033[%dA", (x))			// 上移光标
#define MOVEDOWN(x) 	printf("\033[%dB", (x))			// 下移光标
#define MOVELEFT(y) 	printf("\033[%dD", (y))			// 左移光标
#define MOVERIGHT(y) 	printf("\033[%dC",(y))			// 右移光标
#define MOVETO(x,y) 	printf("\033[%d;%dH", (x), (y))	// 定位光标
#define RESET_CURSOR() 	printf("\033[H")				// 光标复位
#define HIDE_CURSOR() 	printf("\033[?25l")				// 隐藏光标
#define SHOW_CURSOR() 	printf("\033[?25h")				// 显示光标

#define CLRLINE         "\r\e[K"						// 清除一行


// 颜色控制

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

// 背景色
#define BACKGROUND_BLACK             	"\033[40m"
#define BACKGROUND_RED               	"\033[41m"
#define BACKGROUND_GREEN             	"\033[42m"
#define BACKGROUND_YELLOW            	"\033[43m"
#define BACKGROUND_BLUE              	"\033[44m"
#define BACKGROUND_PURPLE            	"\033[45m"
#define BACKGROUND_CYAN              	"\033[46m"
#define BACKGROUND_WHITE             	"\033[47m"
                                          
#define BOLD                 			"\033[1m"		// 高亮
#define UNDERLINE            			"\033[4m"		// 下划线
#define BLINK                			"\033[5m"		// 闪烁
#define REVERSE              			"\033[7m"		// 反白显示
#define HIDE                 			"\033[8m"		// 不可见



#define NO_ENOUGH_PARAM					printf(FOREGROUND_L_RED "%s: No enough parameter\r\n" FOREGROUND_NONE, CMD)
#define PARAM_ERROR						printf(FOREGROUND_L_RED "%s: Parameter error\r\n" FOREGROUND_NONE, CMD)
#define PARAM_NOTSUPPORT				printf(FOREGROUND_L_RED "%s: Parameters that are not yet supported\r\n" FOREGROUND_NONE, CMD)
#define PARAM_TOOMANY					printf(FOREGROUND_L_RED "%s: Too many arguments\r\n" FOREGROUND_NONE, CMD)

#define isParamOver						paramTooMany(CMD)

static taskStack_t cliTaskEnv[CLI_TASK_ENV_SIZE];
static task_t CLITask;
static char CMDPromateBuffer[CLI_CMD_PORMATE_SIZE];

#ifdef MONITOR_FUNCTION
extern task_t* allTask[16];
extern uint32_t allTaskTableIndex;
#endif

static void showWelcome(void) {
	printf(FOREGROUND_YELLOW " __        __  _____   _        ____    ___    __  __   _____     _____    ___      _____                 ____    _____    ___    ____  \r\n" FOREGROUND_NONE);
	printf(FOREGROUND_YELLOW " \\ \\      / / | ____| | |      / ___|  / _ \\  |  \\/  | | ____|   |_   _|  / _ \\    |__  /   ___   _ __   |  _ \\  |_   _|  / _ \\  / ___| \r\n" FOREGROUND_NONE);
	printf(FOREGROUND_YELLOW "  \\ \\ /\\ / /  |  _|   | |     | |     | | | | | |\\/| | |  _|       | |   | | | |     / /   / _ \\ | '_ \\  | |_) |   | |   | | | | \\___ \\ \r\n" FOREGROUND_NONE);
	printf(FOREGROUND_YELLOW "   \\ V  V /   | |___  | |___  | |___  | |_| | | |  | | | |___      | |   | |_| |    / /_  |  __/ | | | | |  _ <    | |   | |_| |  ___) |\r\n" FOREGROUND_NONE);
	printf(FOREGROUND_YELLOW "    \\_/\\_/    |_____| |_____|  \\____|  \\___/  |_|  |_| |_____|     |_|    \\___/    /____|  \\___| |_| |_| |_| \\_\\   |_|    \\___/  |____/ \r\n" FOREGROUND_NONE);
	printf(FOREGROUND_YELLOW "======================================================================================================================================== \r\n" FOREGROUND_NONE);
}



/*
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
*/

static const char spaceCh[] = {KEYBOARD_SPACE, KEYBOARD_CR, KEYBOARD_LR, KEYBOARD_TAB, '\0'};

static uint8_t paramTooMany(const char* CMD) {
	if (strtok(NULL, spaceCh) != NULL) {
		PARAM_TOOMANY;
		return 1;
	}
	return 0;
}

static void showCh(const char ch) {
	uartWrite(&ch, 1);
}

static void unknownCMD(const char* CMD) {
	printf("%s: command not found\r\n", CMD);
}

static void CMD_HELP(const char* CMD) {
	char *type = strtok(NULL, spaceCh);
	
	if (type != NULL) {
		PARAM_TOOMANY;
		return;
	}
	
	printf("+ 使用说明\r\n	- 输入“help”可获取ZenRTOS的CLI使用说明\r\n	- 本CLI支持正常的输入、删除、错误信息提示\r\n\r\n");
	printf("+ 清屏命令\r\n	- 输入“clear”可实现清屏，不保留历史信息\r\n	- 输入“cls”切换到新的一页，保留历史信息\r\n\r\n");
	printf("+ 外部I/O控制\r\n	- 输入“exio set 端口 high/low”可设置对应端口的输出电压\r\n	- 输入“exio get 端口”可获取对应端口电压\r\n	- 输入“exio dir 端口”可设置对应端口输入输出方向\r\n\r\n");
	printf("+ 系统硬件信息获取\r\n	- 输入“cpu info”可获取CPU、系统相关信息\r\n	- 输入“cpu usage”可获取实时CPU利用率\r\n	- 输入“cpu temp”可获取实时CPU温度\r\n\r\n");
	printf("+ 软件信息\r\n	- 输入“lsos”可获取ZenRTOS的软件信息\r\n\r\n");
	printf("+ 任务管理\r\n	- 输入“task info”可获取系统内所有的任务信息\r\n	- 输入“task suspend 任务”可将对应的任务挂起\r\n	- 输入“task delay 任务 时间”可将对应任务延时对应时间\r\n	- 输入“task wakeup 任务”可将对应被挂起的任务唤醒\r\n\r\n");
	printf("+ 系统资源实时监控\r\n	- 输入“monitor on”可进入监控模式，实时查看系统内资源使用情况\r\n\r\n");
	printf("+ 低功耗模式\r\n	- 输入“low-power on”可打开低功耗模式\r\n	- 输入“low-power off”可以关闭低功耗模式\r\n\r\n");
	printf("+ 外部事件检测\r\n	- 按下开发板上按键可被RTOS检测并做出反应\r\n\r\n");
	printf("+ 板上资源控制\r\n	- 输入“beep on/off”可打开或关闭beep\r\n	- 输入“led1/2 on/off”可打开或关闭对应的led灯\r\n	- 输入“light”可获取环境光强度\r\n\r\n");
	printf("+ 软定时器示例\r\n	- 输入“beep squareware 半周期”可利用beep产生对应周期的方波\r\n\r\n");
}

static void CMD_LSOS(const char* CMD) {
	char *type = strtok(NULL, spaceCh);
	
	if (type != NULL) {
		PARAM_TOOMANY;
		return;
	}
	
	printf("ZenRTOS version v1.1\r\n");
	
	printf("SysTick cycle: " FOREGROUND_L_BLUE "%d us\r\n" FOREGROUND_NONE, SYS_TICK);
	
	printf("Task time slice: " FOREGROUND_L_BLUE "%d us \r\n" FOREGROUND_NONE, TIME_SLICE);
	
	printf("\r\nHigh real-time mode: ");		
	
#if HIGH_RT_MODE == 1
	printf(FOREGROUND_L_BLUE "ON\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "OFF\r\n" FOREGROUND_NONE);
#endif
	
	printf("Semaphore: ");

#if FUNCTION_SEMAPHORE_ENABLE == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n" FOREGROUND_NONE);
#endif
	
	printf("Mutex: ");
	
#if FUNCTION_MUTEX_EBABLE == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n" FOREGROUND_NONE);
#endif

	printf("Event flag groups: ");
	
#if FUNCTION_EFLAGGROUP_EBABLE == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n" FOREGROUND_NONE);
#endif

	printf("Mailbox: ");
	
#if FUNCTION_MBOX_ENABLE == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n" FOREGROUND_NONE);
#endif


	printf("Memory blocks: ");
	
#if FUNCTION_MBLOCK_ENABLE == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n" FOREGROUND_NONE);
#endif


	printf("Software Timer: ");
	
#if FUNCTION_SOFTTIMER_ENABLE == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n" FOREGROUND_NONE);
#endif

	printf("CPU Usage: ");
	
#if FUNCTION_CPUUSAGE_ENABLE == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n" FOREGROUND_NONE);
#endif

	printf("Hooks functions:  ");
	
#if FUNCTION_HOOKS_ENABLE == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n" FOREGROUND_NONE);
#endif

	printf("Low-power mode: ");
	
#if LOW_POWER_MODE == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n" FOREGROUND_NONE);
#endif

	printf("FPU: ");
	
#if CORTEX_M4_FPU_OPENED == 1
	printf(FOREGROUND_L_BLUE "ENABLE\r\n\r\n" FOREGROUND_NONE);
#else
	printf(FOREGROUND_RED "DISABLE\r\n\r\n" FOREGROUND_NONE);
#endif
	
	
	
}

static void CMD_CPU(const char* CMD) {
	char *type = strtok(NULL, spaceCh);
	
	if (type == NULL) {
		NO_ENOUGH_PARAM;
		return;
	}
	
	char *typetest = strtok(NULL, spaceCh);
	
	if (typetest != NULL) {
		PARAM_TOOMANY;
		return;
	}
	
	if (strcmp(type, "info") == 0) {
		printf("\r\n");
		printf("SoC: "FOREGROUND_L_PURPLE"STM32F407ZGT6\r\n"FOREGROUND_NONE);
		printf("CPU Core: "FOREGROUND_L_PURPLE"ARM Cortex-M4  "FOREGROUND_NONE);
#if CORTEX_M4_FPU_OPENED == 1
		printf("FPU: "FOREGROUND_RED"ENABLED\r\n"FOREGROUND_NONE);
#else
		printf("FPU: "FOREGROUND_RED"DISABLED\r\n"FOREGROUND_NONE);
#endif
#if LOW_POWER_MODE == 1
		if (LowPowerMode) printf("Low-power mode: "FOREGROUND_L_BLUE"ON\r\n"FOREGROUND_NONE);
		else printf("Low-power mode: "FOREGROUND_RED"OFF\r\n"FOREGROUND_NONE);
#else
		printf("Low-power mode: "FOREGROUND_RED"Unsupported\r\n"FOREGROUND_NONE);
#endif
		printf("Flash capacity: "FOREGROUND_L_PURPLE"1024KB\r\n"FOREGROUND_NONE);
		printf("Internal SRAM capacity: "FOREGROUND_L_PURPLE"192KB\r\n"FOREGROUND_NONE);
		printf("Extend SRAM: "FOREGROUND_L_PURPLE"XM8A51216"FOREGROUND_NONE"  Capacity: "FOREGROUND_L_PURPLE"1MB\r\n"FOREGROUND_NONE);
		printf("\r\n");
		return;
	}
	
	if (strcmp(type, "usage") == 0) {
#if	LOW_POWER_MODE == 1
		if (LowPowerMode) {
			printf(FOREGROUND_BROWN "%s: In low-power mode, this function is not available\r\n" FOREGROUND_NONE, CMD);
			return;
		}
#endif
		
#if FUNCTION_CPUUSAGE_ENABLE == 0
		printf(FOREGROUND_BROWN "%s: function of 'CPU usage' is not enabled in file 'rtConfig'\r\n" FOREGROUND_NONE, CMD);
		printf(FOREGROUND_L_BLUE "Tips: You can enable this function in the 'rtConfig' file and compile again to support it\r\n" FOREGROUND_NONE);
		return;
#endif
		
		printf("CPU利用率: ");
		
		int usage = cpuGetUsage() * 100;
		
		
		if (usage > 7000) {
			printf(FOREGROUND_L_RED);
		} else {
			printf(FOREGROUND_L_GREEN);
		}
		
		printf("%d.%d%%", usage / 100, usage % 100);
		
		printf(FOREGROUND_NONE "\r\n");
		
		return;
	}
	
	if (strcmp(type, "temp") == 0) {
		int temperature = interTemperGetTemperature();
		
		if (temperature < 5000) {
			printf("CPU温度: "FOREGROUND_L_GREEN"%d.%d°C\r\n"FOREGROUND_NONE, temperature / 100, temperature % 100);
		} else if (temperature > 5000) {
			printf("CPU温度: "FOREGROUND_L_RED"%d.%d°C\r\n"FOREGROUND_NONE, temperature / 100, temperature % 100);
		}
		
		return;
	}
	PARAM_ERROR;
}

static void CMD_TASK(const char* CMD) {
	char *type = strtok(NULL, spaceCh);
	
	if (type == NULL) {
		NO_ENOUGH_PARAM;
		return;
	}
	
	if (strcmp(type, "info") == 0) {
		
		if (isParamOver) {
			return;
		}
		
		monitorGetTaskInfo();
		
		return;
	}
	
	if (strcmp(type, "suspend") == 0) {
		type = strtok(NULL, spaceCh);
		
		if (type == NULL) {
			NO_ENOUGH_PARAM;
			return;
		}
		
		if (isParamOver) {
			return;
		}
		
		if (strcmp(type, "MONITOR") == 0) {
			printf(FOREGROUND_L_RED "WARNING: Dangerous operation is not allowed to be performed! \r\n" FOREGROUND_NONE);
			printf(FOREGROUND_L_BLUE "Tips: Task 'MONITOR' can only be scheduled by interface of 'monitor'\r\n" FOREGROUND_NONE);
			return;
		}
		
		if (strcmp(type, "IDLETASK") == 0) {
			printf(FOREGROUND_L_RED "WARNING: IDLETASK is the must component of ZenRTOS, you can't suspend it!\r\n" FOREGROUND_NONE);
			return;
		}
		
		if (strcmp(type, "CMD") == 0) {
			printf(FOREGROUND_L_RED "WARNING: 'CMD' will be suspended, you won't be able to use the command line!\r\n" FOREGROUND_NONE);
			printf(FOREGROUND_L_RED "Are you sure you want to do that? (Y/N)\r\n" FOREGROUND_NONE);
			char ch;
			uartRead(&ch, 1);
			if (ch != 'y' && ch != 'Y') {
				printf(FOREGROUND_L_BLUE "%s: Task 'CMD' is not suspended\r\n" FOREGROUND_NONE, CMD);
				return;
			} else {
				printf("'CMD' is suspended, goodbyd!\r\n");
			}
		}
		
		for (int i = 0; i < allTaskTableIndex; i++) {
			if (allTask[i] != NULL) {
				if (strcmp(type, allTask[i]->taskName) == 0) {
					if (!(allTask[i]->state & TASK_STATUS_DELAY) && !(allTask[i]->state & TASK_STATUS_READY)) {
						printf(FOREGROUND_L_RED "%s: Task '%s' does not have the conditions to be suspended! \r\n" FOREGROUND_NONE, CMD, type);
						return;
					}
					
					uint32_t isDelay = 0;
					
					if (allTask[i]->state & TASK_STATUS_DELAY) {
						printf(FOREGROUND_L_RED "WARNING: Dangerous operation, are you sure you want to do this? (Y/N) \r\n" FOREGROUND_NONE);
						char ch;
						uartRead(&ch, 1);
						if (ch == 'y' || ch == 'Y') {
							isDelay = 1;
							lockSched();
							taskSched2Undelay(allTask[i]);
						} else {
							printf(FOREGROUND_L_BLUE "%s: Task '%s' is not suspended\r\n" FOREGROUND_NONE, CMD, type);
							return;
						}
					}
					
					taskSuspend(allTask[i]);
					
					if (isDelay) unlockSched();
					
					printf(FOREGROUND_L_BLUE "%s: Task '%s' has been suspended\r\n" FOREGROUND_NONE, CMD, type);
					return;
				}
			}
		}
		printf(FOREGROUND_L_RED "%s: There is no such task \r\n" FOREGROUND_NONE, CMD);
		return;
	}
	
	if (strcmp(type, "delay") == 0) {
		type = strtok(NULL, spaceCh);
		
		if (type == NULL) {
			NO_ENOUGH_PARAM;
			return;
		}
		
		char* delayTime = strtok(NULL, spaceCh);
		
		if (isParamOver) {
			return;
		}
		
		int temp = atoi(delayTime);
		
		if (temp <= 0) {
			printf(FOREGROUND_L_RED "%s: The entered delay period is invalid \r\n" FOREGROUND_NONE, CMD);
			return;
		}
		
		if (strcmp(type, "MONITOR") == 0) {
			printf(FOREGROUND_L_RED "WARNING: Dangerous operation is not allowed to be performed! \r\n" FOREGROUND_NONE);
			printf(FOREGROUND_L_BLUE "Tips: Task 'MONITOR' can only be scheduled by interface of 'monitor'\r\n" FOREGROUND_NONE);
			return;
		}
		
		if (strcmp(type, "IDLETASK") == 0) {
			printf(FOREGROUND_L_RED "WARNING: IDLETASK is the must component of ZenRTOS, you can't delay it!\r\n" FOREGROUND_NONE);
			return;
		}
		
		if (strcmp(type, "CMD") == 0) {
			printf(FOREGROUND_L_RED "WARNING: 'CMD' will be delayed, you won't be able to use the command line for %d ms!\r\n" FOREGROUND_NONE, temp);
			printf(FOREGROUND_L_RED "Are you sure you want to do that? (Y/N)\r\n" FOREGROUND_NONE);
			char ch;
			uartRead(&ch, 1);
			if (ch != 'y' && ch != 'Y') {
				printf(FOREGROUND_L_BLUE "%s: Task 'CMD' is not delayed\r\n" FOREGROUND_NONE, CMD);
				return;
			} else {
				printf("'CMD' is delayed\r\n");
			}
		}
		
		for (int i = 0; i < allTaskTableIndex; i++) {
			if (allTask[i] != NULL) {
				if (strcmp(type, allTask[i]->taskName) == 0) {
					if (!(allTask[i]->state & TASK_STATUS_DELAY) && !(allTask[i]->state & TASK_STATUS_READY)) {
						printf(FOREGROUND_L_RED "%s: Task '%s' does not have the conditions to be delayed! \r\n" FOREGROUND_NONE, CMD, type);
						return;
					}
					
					taskDelayOtherTask(allTask[i], temp);
					
					printf(FOREGROUND_L_BLUE "%s: Task '%s' has been delayed for %d ms\r\n" FOREGROUND_NONE, CMD, type, temp);
					return;
				}
			}
		}
		printf(FOREGROUND_L_RED "%s: There is no such task \r\n" FOREGROUND_NONE, CMD);
		return;
	}
	
	
	if (strcmp(type, "wakeup") == 0) {
		type = strtok(NULL, spaceCh);
		
		if (type == NULL) {
			NO_ENOUGH_PARAM;
			return;
		}
		
		if (isParamOver) {
			return;
		}
		
		if (strcmp(type, "MONITOR") == 0) {
			printf(FOREGROUND_L_RED "WARNING: Dangerous operation is not allowed to be performed! \r\n" FOREGROUND_NONE);
			printf(FOREGROUND_L_BLUE "Tips: Task 'MONITOR' can only be scheduled by interface of 'monitor'\r\n" FOREGROUND_NONE);
			return;
		}
		
		for (int i = 0; i < allTaskTableIndex; i++) {
			if (allTask[i] != NULL) {
				if (strcmp(type, allTask[i]->taskName) == 0) {
					if (!(allTask[i]->state & TASK_STATUS_SUSPEND)) {
						printf(FOREGROUND_L_RED "%s: You can't wake up a task that isn't suspended! \r\n" FOREGROUND_NONE, CMD);
						return;
					}
					
					taskWakeUp(allTask[i]);
					
					printf(FOREGROUND_L_BLUE "%s: Task '%s' has been woken up\r\n" FOREGROUND_NONE, CMD, type);
					return;
				}
			}
		}
		printf(FOREGROUND_L_RED "%s: There is no such task \r\n" FOREGROUND_NONE, CMD);
		return;
	}
	
	if ((strcmp(type, "delay") == 0) || (strcmp(type, "block") == 0) || (strcmp(type, "delete") == 0) || (strcmp(type, "destory") == 0)) {
		printf(FOREGROUND_L_RED "WARNING: Dangerous operation is not allowed to be performed! \r\n" FOREGROUND_NONE);
		return;
	}
	PARAM_ERROR;
	
}


static void CMD_MONITOR(const char* CMD) {
	char *type = strtok(NULL, spaceCh);
	
	if (type == NULL) {
		NO_ENOUGH_PARAM;
		return;
	}
	
	if (isParamOver) {
		return;
	}
	
	if (strcmp(type, "on") == 0) {
		monitorOn();
		return;
	}
	
	PARAM_ERROR;
}


static void CMD_LOWPOWERMODE(const char* CMD) {
	char *type = strtok(NULL, spaceCh);
	
	if (type == NULL) {
		NO_ENOUGH_PARAM;
		return;
	}
	
#if LOW_POWER_MODE == 1
	
	if (isParamOver) {
		return;
	}
	
	if (strcmp(type, "on") == 0) {
		printf(FOREGROUND_L_RED "WARNING: Some unforeseen mistakes may happen, are you sure you want to do this? (Y/N) \r\n" FOREGROUND_NONE);
		char ch;
		uartRead(&ch, 1);
		if (ch == 'Y' || ch == 'y') {
			enableLowPower;
			printf(FOREGROUND_L_BLUE "Low power mode is on\r\n" FOREGROUND_NONE);
			printf(FOREGROUND_BROWN "ATTENTION: Some unforeseen mistakes may happen\r\n" FOREGROUND_NONE);
			printf(FOREGROUND_L_BLUE "Tips: In this mode, you need to press the reset button continuously for burning\r\n" FOREGROUND_NONE);
			printf(FOREGROUND_L_BLUE "Tips: In this mode, function of 'cpu usage' is not available\r\n" FOREGROUND_NONE);
		} else {
			printf(FOREGROUND_L_BLUE "%s: low-power mode is not turned on \r\n" FOREGROUND_NONE, CMD);
		}
		return;
	}
	
	if (strcmp(type, "off") == 0) {
		disableLowPower;
		printf(FOREGROUND_L_BLUE "Low power mode is off\r\n" FOREGROUND_NONE);
		return;
	}
	
	if (strcmp(type, "state") == 0) {
		if (LowPowerMode) printf(FOREGROUND_L_BLUE "%s: Low power mode is on\r\n" FOREGROUND_NONE, CMD);
		else printf(FOREGROUND_L_BLUE "%s: Low power mode is off\r\n" FOREGROUND_NONE, CMD);
		return;
	}
#else
	if (isParamOver) {
		unknownCMD(CMD);
		return;
	}
	printf(FOREGROUND_BROWN "ERROR: The low-power mode is not an enabled function\r\n" FOREGROUND_NONE);
	printf(FOREGROUND_L_BLUE "Tips: You can enable this function in the 'rtConfig' file and compile again to support it\r\n" FOREGROUND_NONE);
	return;
#endif
	
	PARAM_ERROR;
}

static void CMD_EXIO(const char* CMD) {
	char *type = strtok(NULL, spaceCh);
	
	if (type == NULL) {
		NO_ENOUGH_PARAM;
		return;
	}
	
	if (strcmp(type, "get") == 0) {
		type = strtok(NULL, spaceCh);
		
		if (isParamOver) {
			return;
		}
		
		if (type == NULL) {
			NO_ENOUGH_PARAM;
		} else if (strstr(type, "PF")) {
			int temp = atoi(&type[2]);
			if (temp == 8 || temp == 9 || temp == 10) {
				IOState st = ExIOGetState((IOType)temp);
				if (st) printf(FOREGROUND_L_BLUE "%s: PF%d: High level\r\n" FOREGROUND_NONE , CMD, temp);
				else printf(FOREGROUND_L_BLUE "%s: PF%d: Low level\r\n" FOREGROUND_NONE, CMD, temp);
			} else {
				PARAM_NOTSUPPORT;
			}
		} else {
			PARAM_NOTSUPPORT;
		}
	} else if (strcmp(type, "set") == 0) {
		type = strtok(NULL, spaceCh);
		if (type == NULL) {
			NO_ENOUGH_PARAM;
		} else if (strstr(type, "PF")) {
			int temp = atoi(&type[2]);
			if (temp == 8 || temp == 9 || temp == 10) {
				type = strtok(NULL, spaceCh);
				
				if (isParamOver) {
					return;
				}
				
				if (strcmp(type, "high") == 0) {
					PFout(temp) = 1;
					printf(FOREGROUND_L_BLUE "%s: PF%d has been set to high level\r\n" FOREGROUND_NONE, CMD, temp);
				} else if(strcmp(type, "low") == 0) {
					PFout(temp) = 0;
					printf(FOREGROUND_L_BLUE "%s: PF%d has been set to low level\r\n" FOREGROUND_NONE, CMD, temp);
				} else {
					PARAM_ERROR;
				}
			} else {
				PARAM_NOTSUPPORT;
			}
		} else {
			PARAM_NOTSUPPORT;
		}
	} else if (strcmp(type, "dir") == 0) {
		
		if (isParamOver) {
			return;
		}
		
		PARAM_NOTSUPPORT;
	} else {
		PARAM_ERROR;
	}
}


static timer_t timerForBeepWare;
static void timerFuncForBeep(void* param) {
	BEEP ^= 1;
}

static void CMD_BEEP(const char* CMD) {
	char* type = strtok(NULL, spaceCh);
	if (type == NULL) {
		NO_ENOUGH_PARAM;
		return;
	}
	
	if (strcmp(type, "on") == 0) {
		
		if (isParamOver) {
			return;
		}
		
		PFout(IO_BEEP) = 1;
		printf(FOREGROUND_L_BLUE "%s: BEEP has been turned on\r\n" FOREGROUND_NONE, CMD);
		return;
	} 
	
	if (strcmp(type, "off") == 0) {
		
		if (isParamOver) {
			return;
		}
		
		PFout(IO_BEEP) = 0;
		printf(FOREGROUND_L_BLUE "%s: BEEP has been turned off\r\n" FOREGROUND_NONE, CMD);
		return;
	}
	
	if (strcmp(type, "state") == 0) {
		
		if (isParamOver) {
			return;
		}
		
		int st = PFout(8);
		if (st) printf(FOREGROUND_L_BLUE "%s: BEEP is on\r\n" FOREGROUND_NONE, CMD);
		else printf(FOREGROUND_L_BLUE "%s: BEEP is off\r\n" FOREGROUND_NONE, CMD);
		return;
	}
	
	if (strcmp(type, "squareware") == 0) {
		type = strtok(NULL, spaceCh);
		
		if (type == NULL) {
			NO_ENOUGH_PARAM;
			return;
		}
		
		if (isParamOver) {
			return;
		}
		
		int temp = atoi(type);
		
		if (temp < 500 && temp > 0) {
			timerInit(&timerForBeepWare, 0, temp, timerFuncForBeep, NULL, TIMER_CONFIG_TYPE_STRICT);
			timerStart(&timerForBeepWare);
			printf(FOREGROUND_L_BLUE "%s: OK\r\n" FOREGROUND_NONE, CMD);
			return;
		}
			
		if (strcmp(type, "close") == 0) {
			timerStop(&timerForBeepWare);
			BEEP = 0;
			printf(FOREGROUND_L_BLUE "%s: OK\r\n" FOREGROUND_NONE, CMD);
			return;
		}
	}
	
	PARAM_ERROR;
		
}

static void CMD_LED(const char* CMD) {
	int temp = atoi(&CMD[3]);
	
	if ((temp != 1) && (temp != 2)) {
		unknownCMD(CMD);
		return;
	}
	
	char* type = strtok(NULL, spaceCh);
	if (type == NULL) {
		NO_ENOUGH_PARAM;
		return;
	}
	
	if (isParamOver) {
		return;
	}
	
	if (strcmp(type, "on") == 0) {
		if (temp == 1) PFout(IO_LED1) = 0;
		else PFout(IO_LED2) = 0;
		printf(FOREGROUND_L_BLUE "led: LED%d has been turned on\r\n" FOREGROUND_NONE, temp);
	} else if (strcmp(type, "off") == 0) {
		if (temp == 1) PFout(IO_LED1) = 1;
		else PFout(IO_LED2) = 1;
		printf(FOREGROUND_L_BLUE "led: LED%d has been turned off\r\n" FOREGROUND_NONE, temp);
	} else if (strcmp(type, "state") == 0) {
		int st = PFout((temp + 8));
		if (st) printf(FOREGROUND_L_BLUE "led: LED%d is off\r\n" FOREGROUND_NONE, temp);
		else printf(FOREGROUND_L_BLUE "led: LED%d is on\r\n" FOREGROUND_NONE, temp);
	} else {
		PARAM_ERROR;
	}
}

static void CMD_LIGHT(const char* CMD) {
	uint8_t lightValue = lightGetValue();
	printf("光强度: %d\r\n", lightValue);
}

static void showPromote() {
	printf("%s ", CMDPromateBuffer);
}

static void readCMD(char* buffer, uint32_t maxlen) {
	uint32_t index = 0;
	uint32_t cursorIndex = 0;
	while (index < maxlen) {
		char ch;
		uartRead(&ch, 1);
RE:		switch (ch) {
			case KEYBOARD_DIR1:
				uartRead(&ch, 1);
				if (ch == KEYBOARD_DIR2) {
					uartRead(&ch, 1);
					switch (ch) {
						case KEYBOARD_DOWN:
						case KEYBOARD_UP:
						case KEYBOARD_LEFT:
						case KEYBOARD_RIGHT:
							break;
						default:
							goto RE;
					}
				} else {
					goto RE;
				}
				break;
			case KEYBOARD_TAB:
				break;
			case KEYBOARD_DELETE:
			case KEYBOARD_BS:
				if (index > 0) {
					buffer[--index] = '\0';
					showCh(KEYBOARD_BS);
					showCh(KEYBOARD_SPACE);
					showCh(KEYBOARD_BS);
					cursorIndex--;
				}
				break;
			case KEYBOARD_CR:
				showCh(KEYBOARD_LR);
			default:
				showCh(ch);
				if ((ch == '\n') || (ch == '\r') || index >= maxlen) {
					buffer[index] = '\0';
					return;
				}
				buffer[index++] = ch;
				cursorIndex++;
				break;
		}
	}
	
}



static void processCMD(char* CMD) {
	char* cmdStart = strtok(CMD, spaceCh);
	
	if (cmdStart == NULL) {
		return;
	}
	
	if(strcmp(cmdStart, "exio") == 0) {
		CMD_EXIO(cmdStart);
		return;
	}
	
	if(strcmp(cmdStart, "beep") == 0) {
		CMD_BEEP(cmdStart);
		return;
	}
	
	if(strstr(cmdStart, "led")) {
		CMD_LED(cmdStart);
		return;
	}
	
	if (strcmp(cmdStart, "cpu") == 0) {
		CMD_CPU(cmdStart);
		return;
	}
	
	if (strcmp(cmdStart, "cls") == 0) {
		CLS;
		RESET_CURSOR();
		return;
	}
	
	if (strcmp(cmdStart, "clear") == 0) {
		CLEAR;
		showWelcome();
		return;
	}
	
	if (strcmp(cmdStart, "task") == 0) {
		CMD_TASK(cmdStart);
		return;
	}
	
	if (strcmp(cmdStart, "light") == 0) {
		CMD_LIGHT(cmdStart);
		return;
	}
	
	if (strcmp(cmdStart, "monitor") == 0) {
		CMD_MONITOR(cmdStart);
		return;
	}
	
	if (strcmp(cmdStart, "low-power") == 0) {
		CMD_LOWPOWERMODE(cmdStart);
		return;
	}
	
	if (strcmp(cmdStart, "banprintf") == 0) {
		banPrint;
		return;
	}
	
	if (strcmp(cmdStart, "allowprintf") == 0) {
		allowPrint;
		return;
	}
	
	if (strcmp(cmdStart, "help") == 0) {
		CMD_HELP(cmdStart);
		return;
	}
	
	if (strcmp(cmdStart, "lsos") == 0) {
		CMD_LSOS(cmdStart);
		return;
	}
	
		
	unknownCMD(cmdStart);
}

static void taskCLIEntry(void* param) {
	taskDelay(5000);
	showWelcome();
	static char CMDReadBuffer[CLI_READ_BUFFER_SIZE];
	while(1) {
		memset(CMDReadBuffer, 0, sizeof(CMDReadBuffer));
		showPromote();
		readCMD(CMDReadBuffer, sizeof(CMDReadBuffer));
		processCMD(CMDReadBuffer);
	}
}

void CLIInit(void) {
	strcpy(CMDPromateBuffer, FOREGROUND_L_GREEN "ZenRTOS>>" FOREGROUND_NONE);
	taskInit(&CLITask, taskCLIEntry, NULL, cliTaskEnv, CLI_TASK_PRIORITY, sizeof(cliTaskEnv), "CMD");
}






