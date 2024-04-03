#include "rt_hooks.h"
#include "ARMCM3.h"

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
