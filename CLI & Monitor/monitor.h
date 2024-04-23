#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>

#define MONITOR_TASK_PRIORITY		0
#define MONITOR_TASK_ENV_SIZE		512
#define MONITOR_MAX_CMD				10
#define	MONITOR_DEFAULT_TIME		1000


void monitorInit(void);
void monitorOn(void);

void monitorGetTaskInfo(void);

#endif
