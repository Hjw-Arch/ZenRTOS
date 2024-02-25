#include<stdint.h>
#include "ARMCM3.h"

// �������û����������Ȼ���ܣ����ǲ��淶
// ����SysTick��ʱ�жϵ�ʱ�䣨�ж����ڣ������ʱ��Ƭ�� 
void setSysTick(uint32_t ms) {
	SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; // �趨Ԥ��ֵ
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);  //�趨���ȼ�
	SysTick->VAL = 0; //�趨��ʱ��ֵ
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; //ʹ���ں˶�ʱ����������ʱ�жϣ�ʹ��SysTick
}

