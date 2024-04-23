#include "exio.h"
#include "RTOS.h"
#include "bitband.h"
#include "stm32f4xx_gpio.h"	


static void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//使能GPIOF时钟

  //GPIOF9,F10初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOF, &GPIO_InitStructure);//初始化
	
	GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);//GPIOF9,F10设置高，灯灭
}

static void BEEP_Init(void)
{   
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//使能GPIOF时钟
  
  //初始化蜂鸣器对应引脚GPIOF8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //BEEP对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; //下拉
	GPIO_Init(GPIOF, &GPIO_InitStructure); //初始化GPIO
	
	GPIO_ResetBits(GPIOF,GPIO_Pin_8);  //蜂鸣器对应引脚GPIOF8拉低， 
}

void ExIOInit(void) {
	LED_Init();
	BEEP_Init();
}

void ExIOSetState(IOType type, IOState st) {
	PFout(type) = st;
}

IOState ExIOGetState(IOType type) {
	IOState state;
	uint32_t st = enterCritical();
	state = PFout(type);
	leaveCritical(st);
	return state;
}


