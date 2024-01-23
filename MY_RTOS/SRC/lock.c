#include "ARMCM3.h"

uint32_t enterCritical(void) {
	uint32_t status = __get_PRIMASK();
	__disable_irq();
	return status;
}

void leaveCritical(uint32_t status) {
	__set_PRIMASK(status);
}
