#ifndef INTERTEMPERATURE
#define INTERTEMPERATURE

#include <stdint.h>
#include "bitband.h" 

#define ADC_CH5  		5 		 	//ͨ��5	   	    

void interTemperatureInit(void); 				//ADCͨ����ʼ��
double interTemperGetTemperature(void);		//�õ��¶�ֵ


#endif
