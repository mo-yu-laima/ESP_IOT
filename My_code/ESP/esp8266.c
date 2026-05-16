#include "esp8266.h"
#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>


char json[200];
char cmd[300];
char reply[256];


unsigned char esp8266_buf[256];
unsigned char esp8266_buf_cpy[256];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

void Usart_SendString(UART_HandleTypeDef *huart, unsigned char *str, unsigned short len)
{

	unsigned short count = 0;
	
	HAL_UART_Transmit_DMA(huart,(uint8_t *)str,len);

}



void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;

}

void ESP8266_Clear_cpy(void)
{

	memset(esp8266_buf_cpy, 0, sizeof(esp8266_buf_cpy));
	esp8266_cnt = 0;

}


_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0)
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)
	{
		esp8266_cnt = 0;

		return REV_OK;		 		//bug־
	}
		
	esp8266_cntPre = esp8266_cnt;
	
	return REV_WAIT;								//bug־

}




_Bool ESP8266_SendCmd(char *cmd, char *res)
{

	unsigned char timeOut = 50;

	Usart_SendString(&huart1, (unsigned char *)cmd, strlen((const char *)cmd));

	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)
			{
				printf("\nesp8266:\n%s\n",esp8266_buf_cpy);
				ESP8266_Clear_cpy();
				ESP8266_Clear();

				return 0;
			}
		}

		HAL_Delay(10);
	}

	return 1;

}


_Bool ESP8266_Send(char *cmd, char *res)
{
	
	unsigned char timeOut = 20;

	Usart_SendString(&huart1, (unsigned char *)cmd, strlen((const char *)cmd));

	//HAL_Delay (500);

	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)
			{
				printf("\nesp8266:\n%s\n",esp8266_buf);
				ESP8266_Clear();
//				ESP8266_Clear_cpy();

				return 0;
			}
		}
		
		HAL_Delay(10);
	}
	
	return 1;

}


void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];
	
	ESP8266_Clear();
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);
	if(!ESP8266_SendCmd(cmdBuf, ">"))
	{
		Usart_SendString(&huart1, data, len);
	}

}


unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,");
			if(ptrIPD == NULL)
			{
				//printf("\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':');
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
		}
		
		HAL_Delay(5);
	} while(timeOut--);
	
	return NULL;

}


void ESP8266_Init(void)
{
	ESP8266_Clear();		//清除上发指令的缓冲区
	ESP8266_Clear_cpy();	//清除平台下发指令的缓冲区

	//重启模块
	printf("RST\n");
	while(ESP8266_Send(RST, "ready"))
	HAL_Delay(5000);

	HAL_Delay(2000);

	printf("PC:\nAT\n");
	while(ESP8266_Send(AT, "OK"))
	HAL_Delay(1000);
	
	//设置模式
	printf("PC:\nCWMODE\n");
	while(ESP8266_Send(CWMODE, "OK"))
	HAL_Delay(2000);

	//连接网络
	printf("PC:\nCWJAP\n");
	while(ESP8266_Send(WIFI_CMD, "WIFI GOT IP"))
	HAL_Delay(2000);

	//配置平台
	printf("PC:\nMQTTUSERCFG\n");
	while(ESP8266_Send(MQTTUSERCFG, "OK"))
	HAL_Delay(2000);

	//连接平台
	printf("PC:\nMQTTCONN\n");
	while(ESP8266_Send(MQTTCONN, "OK"))
	HAL_Delay(2000);

	//订阅主题
	printf("PC:\nMQTTSUB\n");
	while(ESP8266_Send(MQTTSUB, "OK"))
	HAL_Delay(2000);

	//回应平台下发数据
	printf("PC:\nMQTTSUB_SET\n");
	while(ESP8266_Send(MQTTSUB_SET, "OK"))
	HAL_Delay(2000);

	//连接国家授时中心
	printf("PC:\nCIPSNTPCFG\n");
	while(ESP8266_Send(CIPSNTPCFG, "OK"))
	HAL_Delay(2000);

	HAL_Delay(2000);

	printf("PC:\nESP8266 Init OK\n");

	ESP8266_Clear();
	ESP8266_Clear_cpy();

	LED_ON;
	HAL_Delay(1000);
	LED_OFF;

}

