#include "rng.h"
#include "RTOS.h"
#include "stm32f4xx_rng.h"


//��ʼ��RNG
uint8_t RNGInit(void)
{
	uint16_t retry=0; 
	
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);//����RNGʱ��,����PLL48CLK
	
	RNG_Cmd(ENABLE);	//ʹ��RNG
	
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET&&retry < 10000)	//�ȴ����������
	{
		retry++;
		taskDelay(100);
	}
	if(retry >= 10000) return 1;//���������������������
	return 0;
}
//�õ������
//����ֵ:��ȡ���������
uint32_t RNGGetRandomNum(void)
{	 
	while(RNG_GetFlagStatus(RNG_FLAG_DRDY)==RESET);	//�ȴ����������  
	return RNG_GetRandomNumber();
}

//����[min,max]��Χ�������
int RNGGetRandomRange(int min,int max)
{ 
   return RNGGetRandomNum()%(max-min+1) +min;
}



