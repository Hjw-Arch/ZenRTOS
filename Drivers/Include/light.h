#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

#define LSENS_READ_TIMES	5		//���������������ȡ����,����ô���,Ȼ��ȡƽ��ֵ
 
void lightInit(void);				//��ʼ������������
uint8_t lightGetValue(void);				//��ȡ������������ֵ


#endif
