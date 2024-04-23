#include "button.h"
#include "stm32f4xx_gpio.h"

#define loc_key0		1 << 0
#define loc_key1		1 << 1
#define loc_key2		1 << 2
#define loc_keyup		1 << 3

key_t key;

static void timerFuncForKey(void* arg) {
	static uint32_t pressedKey = 0;
	static uint32_t key_down = 0;
	
	static enum {
		NO_DOWN,
		RECHECK_DOWN,
	}scanSt = NO_DOWN;
	
	key_down = ((!KEY0) << 0) | ((!KEY1) << 1) | ((!KEY2) << 2) | (KEY_UP << 3);
	
	switch (scanSt) {
		case NO_DOWN:
			if (key_down != 0) {
				scanSt = RECHECK_DOWN;
			}
			break;
		case RECHECK_DOWN:
			if (key_down == 0) {
				scanSt = NO_DOWN;
			} else if ((key_down & pressedKey) == 0) {
				break;
			} else {
				eFlagGroupPost(&key.eFlagGroup, key_down & pressedKey, 1);
			}
			break;
		default :
			break;
	}
	pressedKey = key_down;
}

void keyInit(void) {
	eFlagGroupInit(&key.eFlagGroup, 0);

	timerInit(&key.timer, 0, 13, timerFuncForKey, NULL, TIMER_CONFIG_TYPE_STRICT);
	
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOA,GPIOEʱ��
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4; //KEY0 KEY1 KEY2��Ӧ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4
	
	 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//WK_UP��Ӧ����PA0
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;//����
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA0
	
	timerStart(&key.timer);
}


