#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>
#include "rtLib.h"
#include "lock.h"
#include "rtConfig.h"
#include "rt_task.h"
#include "rt_time.h"
#include "rt_event.h"

#if FUNCTION_SEMAPHORE_ENABLE == 1
#include "semaphore.h"
#endif

#if FUNCTION_MBOX_ENABLE == 1
#include "rt_mailbox.h"
#endif

#if FUNCTION_MBLOCK_ENABLE == 1
#include "rt_memblock.h"
#endif

#if FUNCTION_EFLAGGROUP_EBABLE == 1
#include "rt_eflaggroup.h"
#endif

#if FUNCTION_MUTEX_EBABLE == 1
#include "rt_mutex.h"
#endif

#if FUNCTION_SEMAPHORE_ENABLE == 1
#if FUNCTION_SOFTTIMER_ENABLE == 1
#include "rt_timer.h"
#endif
#endif

#if FUNCTION_HOOKS_ENABLE == 1
#include "rt_hooks.h"
#endif

#include "rt_idletask.h"

void appInit(void);


#endif
