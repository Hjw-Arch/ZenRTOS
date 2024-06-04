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

#define CLS				printf("\033[2J")				// ��Ļ��ҳ
#define CLEAR			printf("\033c")					// ��������
#define MOVEUP(x)		printf("\033[%dA", (x))			// ���ƹ��
#define MOVEDOWN(x) 	printf("\033[%dB", (x))			// ���ƹ��
#define MOVELEFT(y) 	printf("\033[%dD", (y))			// ���ƹ��
#define MOVERIGHT(y) 	printf("\033[%dC",(y))			// ���ƹ��
#define MOVETO(x,y) 	printf("\033[%d;%dH", (x), (y))	// ��λ���
#define RESET_CURSOR() 	printf("\033[H")				// ��긴λ
#define HIDE_CURSOR() 	printf("\033[?25l")				// ���ع��
#define SHOW_CURSOR() 	printf("\033[?25h")				// ��ʾ���

#define CLRLINE         "\r\e[K"						// ���һ��


// ��ɫ����

// ǰ��ɫ
#define FOREGROUND_NONE                 "\033[0m"			// Ĭ����ɫ
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

// ����ɫ
#define BACKGROUND_BLACK             	"\033[40m"
#define BACKGROUND_RED               	"\033[41m"
#define BACKGROUND_GREEN             	"\033[42m"
#define BACKGROUND_YELLOW            	"\033[43m"
#define BACKGROUND_BLUE              	"\033[44m"
#define BACKGROUND_PURPLE            	"\033[45m"
#define BACKGROUND_CYAN              	"\033[46m"
#define BACKGROUND_WHITE             	"\033[47m"
                                          
#define BOLD                 			"\033[1m"		// ����
#define UNDERLINE            			"\033[4m"		// �»���
#define BLINK                			"\033[5m"		// ��˸
#define REVERSE              			"\033[7m"		// ������ʾ
#define HIDE                 			"\033[8m"		// ���ɼ�



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
// ���ߺ�������������תΪ�ַ����������ӡ���
static void floatToString(float num, char *str, int n) {
    int intPart = (int)num;  // ��������
    float floatPart = num - intPart;  // С������

    // ������������
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
    str[index] = '\0'; // ����ַ���������

    // ��ת��������
    int i = 0, j = index - 1;
    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }

    // ���С����
    str[index++] = '.';

    // ����С������
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
    str[index] = '\0'; // ����ַ���������
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
	
	printf("+ ʹ��˵��\r\n	- ���롰help���ɻ�ȡZenRTOS��CLIʹ��˵��\r\n	- ��CLI֧�����������롢ɾ����������Ϣ��ʾ\r\n\r\n");
	printf("+ ��������\r\n	- ���롰clear����ʵ����������������ʷ��Ϣ\r\n	- ���롰cls���л����µ�һҳ��������ʷ��Ϣ\r\n\r\n");
	printf("+ �ⲿI/O����\r\n	- ���롰exio set �˿� high/low�������ö�Ӧ�˿ڵ������ѹ\r\n	- ���롰exio get �˿ڡ��ɻ�ȡ��Ӧ�˿ڵ�ѹ\r\n	- ���롰exio dir �˿ڡ������ö�Ӧ�˿������������\r\n\r\n");
	printf("+ ϵͳӲ����Ϣ��ȡ\r\n	- ���롰cpu info���ɻ�ȡCPU��ϵͳ�����Ϣ\r\n	- ���롰cpu usage���ɻ�ȡʵʱCPU������\r\n	- ���롰cpu temp���ɻ�ȡʵʱCPU�¶�\r\n\r\n");
	printf("+ �����Ϣ\r\n	- ���롰lsos���ɻ�ȡZenRTOS�������Ϣ\r\n\r\n");
	printf("+ �������\r\n	- ���롰task info���ɻ�ȡϵͳ�����е�������Ϣ\r\n	- ���롰task suspend ���񡱿ɽ���Ӧ���������\r\n	- ���롰task delay ���� ʱ�䡱�ɽ���Ӧ������ʱ��Ӧʱ��\r\n	- ���롰task wakeup ���񡱿ɽ���Ӧ�������������\r\n\r\n");
	printf("+ ϵͳ��Դʵʱ���\r\n	- ���롰monitor on���ɽ�����ģʽ��ʵʱ�鿴ϵͳ����Դʹ�����\r\n\r\n");
	printf("+ �͹���ģʽ\r\n	- ���롰low-power on���ɴ򿪵͹���ģʽ\r\n	- ���롰low-power off�����Թرյ͹���ģʽ\r\n\r\n");
	printf("+ �ⲿ�¼����\r\n	- ���¿������ϰ����ɱ�RTOS��Ⲣ������Ӧ\r\n\r\n");
	printf("+ ������Դ����\r\n	- ���롰beep on/off���ɴ򿪻�ر�beep\r\n	- ���롰led1/2 on/off���ɴ򿪻�رն�Ӧ��led��\r\n	- ���롰light���ɻ�ȡ������ǿ��\r\n\r\n");
	printf("+ ��ʱ��ʾ��\r\n	- ���롰beep squareware �����ڡ�������beep������Ӧ���ڵķ���\r\n\r\n");
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
		
		printf("CPU������: ");
		
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
			printf("CPU�¶�: "FOREGROUND_L_GREEN"%d.%d��C\r\n"FOREGROUND_NONE, temperature / 100, temperature % 100);
		} else if (temperature > 5000) {
			printf("CPU�¶�: "FOREGROUND_L_RED"%d.%d��C\r\n"FOREGROUND_NONE, temperature / 100, temperature % 100);
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
	printf("��ǿ��: %d\r\n", lightValue);
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






