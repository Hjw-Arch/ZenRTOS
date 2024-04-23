#include "exevent.h"
#include "button.h"
#include "uart.h"
#include "RTOS.h"
#include <stdio.h>

task_t exEventTask;
static taskStack_t exEventTaskEnv[EXEVENT_TASK_ENV_SIZE];

static void ExEventTaskEntry (void* param) {
	uint32_t result;
	while(1) {
		eFlagGroupWait(&key.eFlagGroup, EFLAGGROUP_SET | EFLAGGROUP_ANY | EFLAGGROUP_CLEAR_AFTER, 0xf, &result, 0);
		if (result & 0x1) printf("KEY0 DOWN\r\n");
		if (result & 0x2) printf("KEY1 DOWN\r\n");
		if (result & 0x4) printf("KEY2 DOWN\r\n");
		if (result & 0x8) printf("KEYUP DOWN\r\n");
	}
}

void exEventInit(void) {
	keyInit();
	taskInit(&exEventTask, ExEventTaskEntry, NULL, exEventTaskEnv, EXEVENT_TASK_PRIORITY, sizeof(exEventTaskEnv), "EXEVENT");
}


