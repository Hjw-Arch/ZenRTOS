#include "uart.h"
#include "stm32f4xx_usart.h"
#include "stdio.h"
#include "RTOS.h"
#include "assert.h"

static uint8_t rxBuffer[UART_RXBUFFER_SIZE];
static uint8_t txBuffer[UART_TXBUFFER_SIZE];

static sem_t rxReceivedSem;			// ���ջ������ź���������ʵ�����������������⣬��ʾ���ж��ٸ����ݿ��Խ���
static sem_t txFreeSem;			// ���ͻ������ź���������ʵ������������������, ��ʾ���ж��ٸ��ռ����ʹ��

static uint32_t rxWritePos = 0;
static uint32_t rxReadPos = 0;
static uint32_t txWritePos = 0;
static uint32_t txReadPos = 0;

static uint32_t banPrintf = 0;
static uint8_t isBlockedPrintf;

static void uartConfig(void) {
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = UART_BAUDRATE;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	
	USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������ж�
	NVIC_EnableIRQ(USART1_IRQn);
/*
	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����
*/
	
}

void uartInit(void) {
	uartConfig();
	semInit(&rxReceivedSem, 0, UART_RXBUFFER_SIZE);
	semInit(&txFreeSem, UART_TXBUFFER_SIZE, UART_TXBUFFER_SIZE);
}


void uartRead(char* packet, uint32_t len) {
	
	uint32_t st;
	
	while (len-- > 0) {
		semWait(&rxReceivedSem, 0);
		
		st = enterCritical();
		
		*packet++ = rxBuffer[rxReadPos++];
		if (rxReadPos >= UART_RXBUFFER_SIZE) {
			rxReadPos = 0;
		}
		
		leaveCritical(st);
	}
	
}

void uartWrite(const char* packet, uint32_t len) {
	uint32_t st;
	
	while (len-- > 0) {
		
		st = enterCritical();
		
		if (semGetInfo(&txFreeSem).counter == 0) {
			leaveCritical(st);
			semWait(&txFreeSem, 0);
			st = enterCritical();
		} else {
			semWait(&txFreeSem, 0);
		}
		
		txBuffer[txWritePos++] = *packet++;
		if (txWritePos >= UART_TXBUFFER_SIZE) {
			txWritePos = 0;
		}
		
		leaveCritical(st);
		
		// ������ڷ������ݣ���������
		// ���򴥷��ж�
		if (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET) {
			USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		}
	}
}

void USART1_IRQHandler(void) {
	ITStatus status;
	
	// �����ж�
	status = USART_GetITStatus(USART1, USART_IT_RXNE);
	if (status == SET) {
		uint16_t ch = (uint16_t)USART_ReceiveData(USART1);
		
		semInfo_t info = semGetInfo(&rxReceivedSem);
		
		if (info.counter < info.maxcount) {
			rxBuffer[rxWritePos++] = (uint8_t)ch;
			if (rxWritePos >= UART_RXBUFFER_SIZE) {
				rxWritePos = 0;
			}
			
			semPost(&rxReceivedSem);
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);		// ����־λ����
	}
	
	
	// �����ж�
	status = USART_GetITStatus(USART1, USART_IT_TXE);
	if (status == SET) {
		semInfo_t info = semGetInfo(&txFreeSem);
		
		// �����������Ҫ����
		if (info.counter < info.maxcount) {
			uint8_t ch = txBuffer[txReadPos++];
			if (txReadPos >= UART_TXBUFFER_SIZE) {
				txReadPos = 0;
			}
			
			semPost(&txFreeSem);
			
			USART_SendData(USART1, (uint16_t)ch);

		} else {
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}
		
		USART_ClearITPendingBit(USART1, USART_IT_TXE);		// ����־λ����
	}
	
}


void isBanPrintf(uint8_t isban) {
	banPrintf = isban;
}


// �Ƿ�ʹ��������������ڵķ�ʽ
// ������������׳�������ֵֹ�����
// ������������ܸߣ����ǳ�ʱ������������������������
void blockedPrintf(uint8_t isopen) {
	isBlockedPrintf = isopen;
}


//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	

void _ttywrch(int ch){}

#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{
	if (banPrintf) {
		return 0;
	}
	
	if (isBlockedPrintf) {
		USART_SendData(USART1,(u8)ch);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	} else {
		uartWrite((char*)&ch, 1);
	}
	
	return ch;
}



