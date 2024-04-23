#ifndef UART_H
#define UART_H

#include <stdint.h>

#define UART_BAUDRATE		256000
#define UART_RXBUFFER_SIZE	32
#define UART_TXBUFFER_SIZE	1024


#define EN_USART1_RX		1


void uartInit(void);
void uartRead(char* packet, uint32_t len);
void uartWrite(const char* packet, uint32_t len);
void blockedPrintf(uint8_t isopen);
void isBanPrintf(uint8_t isban);

#define blockedPrint		blockedPrintf(1)
#define unblockedPrint		blockedPrintf(0)

#define allowPrint			isBanPrintf(0)
#define banPrint			isBanPrintf(1)

#endif
