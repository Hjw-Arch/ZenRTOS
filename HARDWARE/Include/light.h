#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

#define LSENS_READ_TIMES	5		//定义光敏传感器读取次数,读这么多次,然后取平均值
 
void lightInit(void);				//初始化光敏传感器
uint8_t lightGetValue(void);				//读取光敏传感器的值


#endif
