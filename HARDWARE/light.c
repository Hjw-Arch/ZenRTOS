#include "light.h"
#include "RTOS.h"
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_adc.h"


//��ʼ��ADC
//�������ǽ��Թ���ͨ��Ϊ��														   
static void adc3Init(void)
{    
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE); //ʹ��ADC3ʱ��

	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3, ENABLE);	  //ADC3��λ
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3, DISABLE);	//��λ����	 
 
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//����ģʽ
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//���������׶�֮����ӳ�5��ʱ��
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMAʧ��
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz 
	ADC_CommonInit(&ADC_CommonInitStructure);//��ʼ��
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12λģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//��ɨ��ģʽ	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//�ر�����ת��
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//��ֹ������⣬ʹ���������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//�Ҷ���	
	ADC_InitStructure.ADC_NbrOfConversion = 1;//1��ת���ڹ��������� Ҳ����ֻת����������1 
	ADC_Init(ADC3, &ADC_InitStructure);//ADC��ʼ��
	
	ADC_Cmd(ADC3, ENABLE);//����ADת����	 
}				  
//���ADCֵ
//ch:ͨ��ֵ 0~16 ADC_Channel_0~ADC_Channel_16
//����ֵ:ת�����
static uint16_t adc3Get(u8 ch)   
{
	  //����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC3, ch, 1, ADC_SampleTime_480Cycles);	//ADC3,ADCͨ��,480������,��߲���ʱ�������߾�ȷ��			    
  
	ADC_SoftwareStartConv(ADC3);		//ʹ��ָ����ADC3�����ת����������	
	 
	while(!ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC));//�ȴ�ת������

	return ADC_GetConversionValue(ADC3);	//�������һ��ADC3�������ת�����
}


//��ʼ������������
void lightInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//ʹ��GPIOFʱ��
	
  //�ȳ�ʼ��ADC3ͨ��7IO��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//PA7 ͨ��7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//ģ������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//����������
	GPIO_Init(GPIOF, &GPIO_InitStructure);//��ʼ��  
 
	adc3Init();//��ʼ��ADC3
}
//��ȡLight Sens��ֵ
//0~100:0,�;100,���� 
uint8_t lightGetValue(void)
{
	uint32_t temp_val=0;
	uint8_t t;
	for(t=0; t < LSENS_READ_TIMES; t++)
	{
		temp_val += adc3Get(ADC_Channel_5);	//��ȡADCֵ,ͨ��5
		taskDelay(10);
	}
	temp_val /= LSENS_READ_TIMES;//�õ�ƽ��ֵ 
	if(temp_val > 4000) temp_val = 4000;
	return (uint8_t)(100 - (temp_val / 40));
}




