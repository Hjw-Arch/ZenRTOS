#include "exio.h"
#include "RTOS.h"
#include "bitband.h"
#include "stm32f4xx_gpio.h"	


static void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//ʹ��GPIOFʱ��

  //GPIOF9,F10��ʼ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOF, &GPIO_InitStructure);//��ʼ��
	
	GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);//GPIOF9,F10���øߣ�����
}

static void BEEP_Init(void)
{   
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//ʹ��GPIOFʱ��
  
  //��ʼ����������Ӧ����GPIOF8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //BEEP��Ӧ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; //����
	GPIO_Init(GPIOF, &GPIO_InitStructure); //��ʼ��GPIO
	
	GPIO_ResetBits(GPIOF,GPIO_Pin_8);  //��������Ӧ����GPIOF8���ͣ� 
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


