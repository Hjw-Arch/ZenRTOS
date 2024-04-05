#include "rt_hooks.h"
#if CORTEX_M4_FPU_OPENED == 1
#include "stm32f4xx.h"
#else
#include "ARMCM3.h"
#endif

#if FUNCTION_HOOKS_ENABLE == 1

void hooksCpuIdle(void) {
//	__WFI();
}

void hooksSysTick(void){
	
}

void hooksTaskSwitch(task_t* from, task_t* to){
	
}

void hooksTaskInit(task_t* task){
	
}



#endif
