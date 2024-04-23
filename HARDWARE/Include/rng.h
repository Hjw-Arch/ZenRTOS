#ifndef RNG_H
#define RNG_H

#include "stdint.h"
	
uint8_t  RNGInit(void);			//RNG初始化 
uint32_t RNGGetRandomNum(void);//得到随机数
int RNGGetRandomRange(int min,int max);//生成[min,max]范围的随机数

#endif





