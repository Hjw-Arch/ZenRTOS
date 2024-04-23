#include<stdint.h>
#include "rtConfig.h"
#if CORTEX_M4_FPU_OPENED == 1
#include "stm32f4xx.h"
#else
#include "ARMCM3.h"
#endif

// 这个函数没有声明，虽然能跑，但是不规范
// 设置SysTick定时中断的时间（中断周期，任务的时间片） 
void setSysTick(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; // 设定预载值
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);  //设定优先级
	SysTick->VAL = 0; //设定定时器值
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; //使用内核定时器；启动定时中断；使能SysTick
}

