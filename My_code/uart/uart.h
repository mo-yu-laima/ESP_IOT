#ifndef __UART_H__
#define __UART_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"


#define RXBUFFERSIZE1  256
char Uart1_RxBuffer[RXBUFFERSIZE1];
uint8_t Uart1_RxData;
uint8_t Uart1_Rx_Cnt = 0;

#define RXBUFFERSIZE3  256
char Uart3_RxBuffer[RXBUFFERSIZE3];
uint8_t Uart3_RxData;
uint8_t Uart3_Rx_Cnt = 0;


#define RXBUFFERSIZE2  256
char Uart2_RxBuffer[RXBUFFERSIZE2];
uint8_t Uart2_RxData;
uint8_t Uart2_Rx_Cnt = 0;


#endif /* __UART_H__ */


