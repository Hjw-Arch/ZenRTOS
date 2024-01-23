#include "ARMCM3.h"

void taskSched(void);

uint8_t schedLockCount = 0;

// ÁÙ½çÇø
uint32_t enterCritical(void) {
	uint32_t status = __get_PRIMASK();
	__disable_irq();
	return status;
}

void leaveCritical(uint32_t status) {
	__set_PRIMASK(status);
}

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




