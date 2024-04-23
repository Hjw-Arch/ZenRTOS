#ifndef BUTTON_H
#define BUTTON_H

#include "RTOS.h"

#define KEY0 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) //PE4
#define KEY1 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)	//PE3 
#define KEY2 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) //PE2
#define KEY_UP 		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)	//PA0

typedef enum {
	key_0 = 0,
	key_1,
	key_2,
	key_UP,
	key_null,
}keyId_t;


typedef struct _key {
	keyId_t key;
	eFlagGroup_t eFlagGroup;
	timer_t timer;
}key_t;

extern key_t key;
void keyInit(void);

#endif
