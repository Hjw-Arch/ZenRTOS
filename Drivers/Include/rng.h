#ifndef RNG_H
#define RNG_H

#include "stdint.h"
	
uint8_t  RNGInit(void);			//RNG��ʼ�� 
uint32_t RNGGetRandomNum(void);//�õ������
int RNGGetRandomRange(int min,int max);//����[min,max]��Χ�������

#endif





