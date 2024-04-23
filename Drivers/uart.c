#include "uart.h"
#include "stm32f4xx_usart.h"
#include "stdio.h"
#include "RTOS.h"
#include "assert.h"

static uint8_t rxBuffer[UART_RXBUFFER_SIZE];
static uint8_t txBuffer[UART_TXBUFFER_SIZE];

static sem_t rxReceivedSem;			// 接收缓冲区信号量，用于实现生产者消费者问题，表示还有多少个数据可以接收
static sem_t txFreeSem;			// 发送缓冲区信号量，用于实现生产者消费者问题, 表示还有多少个空间可以使用

static uint32_t rxWritePos = 0;
static uint32_t rxReadPos = 0;
static uint32_t txWritePos = 0;
static uint32_t txReadPos = 0;

static uint32_t banPrintf = 0;
static uint8_t isBlockedPrintf;

static void uartConfig(void) {
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = UART_BAUDRATE;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
	USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断
	NVIC_EnableIRQ(USART1_IRQn);
/*
	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
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
		
		// 如果正在发送数据，则不做处理
		// 否则触发中断
		if (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET) {
			USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		}
	}
}

void USART1_IRQHandler(void) {
	ITStatus status;
	
	// 接收中断
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
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);		// 将标志位清零
	}
	
	
	// 发送中断
	status = USART_GetITStatus(USART1, USART_IT_TXE);
	if (status == SET) {
		semInfo_t info = semGetInfo(&txFreeSem);
		
		// 如果有数据需要发送
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
		
		USART_ClearITPendingBit(USART1, USART_IT_TXE);		// 将标志位清零
	}
	
}


void isBanPrintf(uint8_t isban) {
	banPrintf = isban;
}


// 是否使用阻塞输出到串口的方式
// 阻塞输出不容易出现奇奇怪怪的问题
// 非阻塞输出性能高，但是长时间大量输出且有输入会出现问题
void blockedPrintf(uint8_t isopen) {
	isBlockedPrintf = isopen;
}


//加入以下代码,支持printf函数,而不需要选择use MicroLIB	

void _ttywrch(int ch){}

#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{
	x = x; 
} 
//重定义fputc函数 
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



