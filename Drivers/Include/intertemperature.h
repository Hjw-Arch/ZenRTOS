#ifndef INTERTEMPERATURE
#define INTERTEMPERATURE

#include <stdint.h>
#include "bitband.h" 

#define ADC_CH5  		5 		 	//通道5	   	    

void interTemperatureInit(void); 				//ADC通道初始化
double interTemperGetTemperature(void);		//得到温度值


#endif
