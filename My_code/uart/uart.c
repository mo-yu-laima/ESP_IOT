/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "usart.h"
#include <stdio.h>
#include "string.h"
#include "esp8266.h"

//printf重定向
#if defined(__GNUC__)
int _write(int fd, char *ptr, int len)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, HAL_MAX_DELAY);

	return len;
}
#endif


extern uint8_t reply_flag;	//答复平台的标志位  0：平台没问候   1：平台问候了，我要开始答复
extern uint8_t utc_flag;	//北京时间配置标志位

extern unsigned short esp8266_cnt;
extern unsigned char esp8266_buf[256];
extern unsigned char esp8266_buf_cpy[256];

extern char gps_buff[100];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
		if(huart == &huart1)	//接收到esp8266的消息
		{

			
			if(esp8266_cnt >= sizeof(esp8266_buf))
			{  
				
				esp8266_cnt = 0;
			}
			
			
			esp8266_buf[esp8266_cnt] = Uart1_RxData;
			esp8266_buf_cpy[esp8266_cnt++] = Uart1_RxData;
			
			if(Uart1_RxData == '\n')	//一个数据帧接收完毕
			{
				if (strstr(esp8266_buf_cpy, "$sys/"DEVICE_ID"/"USER_NAME"/thing/property/set"))	reply_flag = 1;		//检查是否有平台要我们设备应答的消息
				if(strstr(esp8266_buf_cpy,"+CIPSNTPTIME"))  utc_flag = 1;		//检查授时中心下发的北京时间
			}
			
			HAL_UART_Receive_IT(&huart1,(uint8_t *)&Uart1_RxData, 1);
		}
		
		
		
		
		if(huart == &huart3)	//接收到电脑串口助手的消息
		{

			
			if(esp8266_cnt >= sizeof(esp8266_buf))
			{  
				
				esp8266_cnt = 0;
			}
			
			
			esp8266_buf[esp8266_cnt] = Uart3_RxData;
			esp8266_buf_cpy[esp8266_cnt++] = Uart3_RxData;


			
			HAL_UART_Receive_DMA(&huart3,(uint8_t *)&Uart3_RxData, 1);
		}
		

		if(huart == &huart2)		//接收GPS的消息
		{
			LED_ON;
			if(Uart2_RxData == '\n')
			{
				gps_buff[Uart2_Rx_Cnt] = '\0';
				Uart2_Rx_Cnt = 0;
				printf("\n%s\n",gps_buff);
				printf("OK\n");

			}
			else
			{
				gps_buff[Uart2_Rx_Cnt++] = Uart2_RxData;
				//(Uart2_RxData >= 100) ? Uart2_RxData : 0;

			}

			HAL_UART_Receive_IT(&huart2,(uint8_t *)&Uart2_RxData, 1);

		}

}



