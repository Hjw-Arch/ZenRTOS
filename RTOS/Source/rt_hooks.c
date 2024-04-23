#include "rt_hooks.h"
#if CORTEX_M4_FPU_OPENED == 1
#include "stm32f4xx.h"
#else
#include "ARMCM3.h"
#endif

#if FUNCTION_HOOKS_ENABLE == 1

#if LOW_POWER_MODE == 1
uint8_t isenableLowPowerMode = 0;
#endif

void hooksCpuIdle(void) {
#if LOW_POWER_MODE == 1
	if (isenableLowPowerMode) {
		__WFI();
	}
#endif
}

void hooksSysTick(void){
	
}

void hooksTaskSwitch(task_t* from, task_t* to){
	
}

void hooksTaskInit(task_t* task){
	
}



#endif
