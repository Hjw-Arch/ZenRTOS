#include <stdint.h>
#include "rtConfig.h"

#if CORTEX_M4_FPU_OPENED == 1
#include "stm32f4xx.h"
#else
#include "ARMCM3.h"
#endif

void taskSched(void);

uint8_t schedLockCount = 0;

// 临界区
uint32_t enterCritical(void) {
	uint32_t status = __get_PRIMASK();
	__disable_irq();
	return status;
}

void leaveCritical(uint32_t status) {
	__set_PRIMASK(status);
}

// 调度锁
void schedLockInit(void){
	schedLockCount = 0;
}

void lockSched(void) {
	uint32_t st = enterCritical();
	
	if (schedLockCount < 255) schedLockCount++;
	
	leaveCritical(st);
}

void unlockSched(void) {
	uint32_t st = enterCritical();
	
	if (schedLockCount > 0) {
		if (--schedLockCount == 0) {
			taskSched();
		}
	}
	
	leaveCritical(st);
}




