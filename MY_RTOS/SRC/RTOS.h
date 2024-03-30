#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>
#include "rtLib.h"
#include "lock.h"
#include "rtConfig.h"
#include "rt_task.h"
#include "rt_time.h"
#include "rt_event.h"
#include "semaphore.h"
#include "rt_mailbox.h"
#include "rt_memblock.h"
#include "rt_eflaggroup.h"
#include "rt_mutex.h"
#include "rt_timer.h"

#include "rt_idletask.h"

void appInit(void);


#endif
