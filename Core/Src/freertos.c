/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dht11.h"
#include "Light.h"
#include "esp8266.h"
#include "string.h"
#include "stdio.h"
#include "oled.h"
#include "EC11.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
//extern uint8_t Light;

extern char json[200];
extern char cmd[300];
extern char reply[256];
extern uint8_t EC11_flag;		//编码器动作标志位
extern uint8_t Onet_flag;		//OnenetTask中OLED显示数据的标志位

uint8_t reply_flag = 0;
uint8_t utc_flag = 0;
uint8_t Task_flag= 0;	//所有任务运行状态

TimeData time_data;	//存储标准时间的变量

uint8_t month;	//当年前月份


extern unsigned char esp8266_buf[256];
extern unsigned char esp8266_buf_cpy[256];

extern void DHT11Task(void);
extern void LightTask(void);
extern void MPU6050Task(void);

//存储消息队列的缓冲区
SensorData_t tx_buff =
{
	.Data = {1,0,1,0,0},
	.Light = 1
};



/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 288 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for myQueue01 */
osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = {
  .name = "myQueue01"
};
/* Definitions for UTC */
osTimerId_t UTCHandle;
const osTimerAttr_t UTC_attributes = {
  .name = "UTC"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osThreadId_t DHT11TaskHandle;
const osThreadAttr_t DHT11Task_attributes = {
  .name = "DHT11Task",
  .stack_size = 128 * 7,
  .priority = (osPriority_t) osPriorityNormal,
};


osThreadId_t LightTaskHandle;
const osThreadAttr_t LightTask_attributes = {
  .name = "LightTask",
  .stack_size = 128 * 7,
  .priority = (osPriority_t) osPriorityNormal,
};


osThreadId_t OnenetTaskHandle;
const osThreadAttr_t OnenetTask_attributes = {
  .name = "OnenetTask",
  .stack_size = 128 * 11,
  .priority = (osPriority_t) osPriorityNormal+1,
};


osThreadId_t MPU6050TaskHandle;
const osThreadAttr_t MPU6050Task_attributes = {
  .name = "MPU6050Task",
  .stack_size = 128 * 8,
  .priority = (osPriority_t) osPriorityNormal,
};



void OnenetTask();


//将英文的月份转换成阿拉伯数字，因为在授时中心获取到的月份是英文的（week也是英文的）
void Chage_Month()
{
	const char* month_table[] =
	{
	    "Jan","Feb","Mar","Apr","May","Jun",
	    "Jul","Aug","Sep","Oct","Nov","Dec"
	};

	month = 0;
	for(int i=0;i<12;i++)
	{
	    if(strcmp(time_data.month, month_table[i]) == 0)
	    {
	        month = i + 1;
	        break;
	    }
	}

}


extern uint8_t idex;	//EC11顺时针旋转的次数
uint8_t idex_cpy= 0;	//存储上一次idex的次数

//将flash内存储的报警信息在OLED上显示出来
//一页OLED最多显示4条报警信息，可以顺时针旋转EC11来查看下一页的报警信息
void OLED_Flash(void)
{
    uint8_t buf;
    uint32_t addr = 0;
    uint8_t x = 0, y = 0;

    while (1)
    {
    	idex_cpy= idex;

        W25Q64_ReadData(&buf, addr, 1);
//        printf("ADDR:%lu  BYTE:0x%02X '%c'\r\n", addr, buf, buf);

        if (buf == 0xFF)	//获取完所有的报警内容了
        	{
        		EC11_flag= 3;
        		break;
        	}

        if (buf == '\n')	//如果遇到\n就不显示出来，因为OLED显示不了\n，强行显示会乱码
        {
//            x = 0;
//            y += 2;
//            if (y >= 8) break;
            addr++;
            continue;
        }

        OLED_ShowChar(x, y, buf, 1);

        x += 6;
        if (x + 6 >= 128)
        {
            x = 0;
            y += 2;
            if (y >= 8)	//这一页显示满了，想要显示更多数据得换页
            {
            	while(idex == idex_cpy)	//等待EC11顺时针旋转
            		osDelay(500);
        		x= 0;
        		y= 0;
            }
        }

        addr++;
    }
}





/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void UTC_Callback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of UTC */
  UTCHandle = osTimerNew(UTC_Callback, osTimerPeriodic, NULL, &UTC_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of myQueue01 */
  myQueue01Handle = osMessageQueueNew (3, sizeof(tx_buff), &myQueue01_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  //***************************************注意每次更新cubeMX后要把下面的消息队列创造复制覆盖cubemx生成的队列创建代码******/
//  myQueue01Handle = osMessageQueueNew (3, sizeof(tx_buff), &myQueue01_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  OnenetTaskHandle = osThreadNew(OnenetTask, NULL, &OnenetTask_attributes);
  DHT11TaskHandle = osThreadNew(DHT11Task, NULL, &DHT11Task_attributes);
  LightTaskHandle = osThreadNew(LightTask, NULL, &LightTask_attributes);
  MPU6050TaskHandle = osThreadNew(MPU6050Task, NULL, &MPU6050Task_attributes);


  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
	//MX_IWDG_Init();

	OLED_Clear();
	OLED_ShowCHinese(20,2,4);	//校
	OLED_ShowCHinese(40,2,5);	//准
	OLED_ShowCHinese(60,2,6);	//时
	OLED_ShowCHinese(80,2,7);	//间

	printf("PC:\nCIPSNTPTIME\n");	//获取当前时间
	while(ESP8266_Send(CIPSNTPTIME,"OK"))
	HAL_Delay(2000);


	osDelay(2000);
  for(;;)
  {
	  if(utc_flag)
	  {
		  char *p = strstr(esp8266_buf_cpy, "+CIPSNTPTIME:");
		  if (p) {
		      // 找到第一行的结束位置，截断
		      char *end = strstr(p, "\r\n");
		      if (end) *end = '\0';

		      // 现在 p 变成：
		      // "+CIPSNTPTIME:Thu Nov 27 22:52:44 2025"
		      sscanf(p,
		             "+CIPSNTPTIME:%3s %3s %d %d:%d:%d %d",
		             time_data.weekday,
		             time_data.month,
		             &time_data.day,
		             &time_data.hour,
		             &time_data.minute,
		             &time_data.second,
		             &time_data.year);

		      Chage_Month();	//将英文的月份转换成阿拉伯数字

			  ESP8266_Clear_cpy();
		  }

		  if(time_data.year== 1970)	//如果jiaozhun时间失败就一直获取，直到成功（校准时间失败年份会显示1970)
		  {
				printf("PC:\nCIPSNTPCFG\n");
				while(ESP8266_Send(CIPSNTPCFG, "OK"))
				HAL_Delay(2000);

				printf("PC:\nCIPSNTPTIME\n");
				while(ESP8266_Send(CIPSNTPTIME,"OK"))
				HAL_Delay(2000);
		  }
		  else	//校准时间成功
		  {
			  osTimerStart (UTCHandle, 1000);	//计时器开始工作，1秒进一次中断
			  time_data.second+=2;	//因为代码运行需要时间，可能会有几秒的偏差，这里进行校准
			  OLED_Clear();
			  utc_flag = 0;	//校准成功
			  Task_flag= 1;	//所有任务可以开始运行
		  }

	  }

	  if(EC11_flag== 1)	//顺时针旋转了EC11
	  {

		  OLED_Flash();	//显示报警日志
	  }
	  else if(EC11_flag== 2)	//逆时针旋转EC11
	  {
		  OLED_Clear();

		  W25Q64_SectorErase(0);	//清空flash内的报警日志（0...3扇区）
		  W25Q64_SectorErase(1);
		  W25Q64_SectorErase(2);
		  W25Q64_SectorErase(3);

		  EC11_flag= 0;	//EC11标志位恢复初始状态（没有拧过的状态）
		  Onet_flag= 1;	//OnenetTask可以继续在OLED上显示温度等数据
	  }



	//HAL_IWDG_Refresh(&hiwdg);
    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* UTC_Callback function */
void UTC_Callback(void *argument)
{
  /* USER CODE BEGIN UTC_Callback */

	time_data.second++;
	if(time_data.second>= 60) time_data.minute++, time_data.second= 0;
	if(time_data.minute>= 60) time_data.hour++, time_data.minute= 0;
	if(time_data.hour>= 24)	//时间过0点时校准一下时间
	{
		//暂停其他任务，避免串口过载，争夺esp8266的串口口资源
		osThreadSuspend(DHT11TaskHandle);
		osThreadSuspend(LightTaskHandle);
		osThreadSuspend(MPU6050TaskHandle);
		osThreadSuspend(OnenetTaskHandle);

		ESP8266_Clear_cpy();

		printf("PC:\nCIPSNTPTIME\n");
		while(ESP8266_Send(CIPSNTPTIME,"OK"))
		HAL_Delay(2000);

		Onet_flag== 1;

		osThreadResume(DHT11TaskHandle);
		osThreadResume(LightTaskHandle);
		osThreadResume(MPU6050TaskHandle);
		osThreadResume(OnenetTaskHandle);
	}
	if(EC11_flag== 0)	//没有在日志显示状态下，运行显示当前时间
	{
		OLED_ShowNum(60,0,time_data.hour,2,2);
		OLED_ShowNum(78,0,time_data.minute,2,2);
		OLED_ShowNum(96,0,time_data.second,2,2);
	}

  /* USER CODE END UTC_Callback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */



void OnenetTask()
{
//	osThreadSuspend(DHT11TaskHandle);
//	osThreadSuspend(LightTaskHandle);
//	osThreadSuspend(MPU6050TaskHandle);
//	osThreadSuspend(NULL);

	do
	{
		osDelay(1000);
	}while(!Task_flag);	//等待时间校准完毕

	//存储上一次的数据
	uint8_t Hum = 0, Temp = 0, Light_nx = 0;
	int8_t AngleX_nx= 0, AngleY_nx= 0;

	//获取消息队列内的消息的缓存区
	int8_t data[8] = {1,1,1,1,1,1,1,1};

	//消息队列的状态
	osStatus_t sates;

	//访问esp8266超时时间
	uint8_t time_out = 0;

	//存储回复平台的ID号
	char id_buf[32];
	while(1)
	{
		if (reply_flag == 1)	//有需要答复平台的消息
		{
			//防止在答复信息时，数据更新，与当前答复信息竞争esp8266的串口资源
			osThreadSuspend(DHT11TaskHandle);
			osThreadSuspend(LightTaskHandle);
			osThreadSuspend(MPU6050TaskHandle);

			//截取出ID号（因为每次平台下发的ID号 是随机的，我们答复的时候要于平台下发的ID一致才算有效答复
			char *p = strstr(esp8266_buf_cpy, "\"id\":\"");
			if (p)
			{
			    p += strlen("\"id\":\"");        // 跳到 id 起始位置
			    char *q = strchr(p, '"');        // 找到下一个 "
			    if (q)
			    {
			        int len = q - p;
			        if (len > 0 && len < sizeof(id_buf))
			        {
			            strncpy(id_buf, p, len);
			            id_buf[len] = '\0';
			        }
			    }
			}


			if (strstr(esp8266_buf_cpy, "\"LED\":true"))
			{
				LED_ON;
			}
			else if(strstr(esp8266_buf_cpy, "\"LED\":f"))
			{
				LED_OFF;
			}


			sprintf(reply,
			    "AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/set_reply\",\"{\\\"id\\\":\\\"%s\\\"\\\,\\\"code\\\":0\\\,\\\"msg\\\":\\\"success\\\"}\",0,0\r\n",
			    DEVICE_ID,
			    USER_NAME,
				id_buf
			);

			printf("PC:\nMQTTreply\n");		//答复平台
			ESP8266_Clear_cpy();
			while(ESP8266_SendCmd(reply, "OK"))
			{
					time_out++;
					osDelay(1000);
					if(time_out >= 3) break;
			}

			osThreadResume(DHT11TaskHandle);
			osThreadResume(LightTaskHandle);
			osThreadResume(MPU6050TaskHandle);

			reply_flag = 0;		//回复完毕
			ESP8266_Clear_cpy();
		}



		else if(reply_flag == 0)	//如果没有需要应答的消息 就主动去更新数据给云平台
		{
			sates = osMessageQueueGet (myQueue01Handle, data, NULL, 0);		//获取消息队列内的内容


			if(sates == osOK || Onet_flag== 1)	//消息准备号了  或者  刚刚退出日志显示状态
			{

				//暂停其他任务，防止干扰
				osThreadSuspend(DHT11TaskHandle);
				osThreadSuspend(LightTaskHandle);
				osThreadSuspend(MPU6050TaskHandle);

//				printf("%d %d\r\n",data_3[0],data_3[2]);

				//限制更新数据给onenet平台的阈值，节约串口资源，节省时间
				if( data[0] >= Hum+2 || data[0] <= Hum-2 ||data[2] <= Temp-2 || data[2] >= Temp+2 || data[5] >= Light_nx+3 || data[5] <= Light_nx-3 || AngleX_nx >= data[6]+5 || AngleX_nx <= data[6]-5 || AngleY_nx >= data[7]+5 || AngleY_nx <= data[7] -5 || Onet_flag== 1)
				{


					sprintf(json,"{\\\"id\\\":\\\"1\\\"\\\,\\\"params\\\":{\\\"Temp\\\":{\\\"value\\\":%d}\\\,\\\"Hum\\\":{\\\"value\\\":%d}\\\,\\\"Light\\\":{\\\"value\\\":%d}\\\,\\\"AngleX\\\":{\\\"value\\\":%d}\\\,\\\"AngleY\\\":{\\\"value\\\":%d}}}",
						data[2],
						data[0],
						data[5],
						data[6],
						data[7]
					);

					sprintf(cmd,"AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/post\",\"%s\",0,0\r\n",
						DEVICE_ID,
						USER_NAME,
						json
					);


					printf("PC:\nMQTTPUB\n");	//上传数据给平台
					ESP8266_Clear();
					while(ESP8266_Send(cmd, "OK"))
					{
							time_out++;
							osDelay(1000);
							if(time_out >= 2) break;
					}

					if(EC11_flag== 0)	//如果不在日志显示页面时，便显示温度等数据
					{
						OLED_ShowNum(0,0,time_data.year,4,2);
						OLED_ShowChar(25,0,'.',2);
						OLED_ShowNum(28,0,month,2,2);
						OLED_ShowChar(40,0,'.',2);
						OLED_ShowNum(43,0,time_data.day,2,2);


						OLED_ShowChar(73,0,':',2);


						OLED_ShowChar(91,0,':',2);

						OLED_ShowString(0,2,(uint8_t *)"Light:",7);
						OLED_ShowNum(50,2,data[5],2,2);

						OLED_ShowString(0,4,(uint8_t *)"Hum:",5);
						OLED_ShowNum(50,4,data[0],2,2);

						OLED_ShowString(0,6,(uint8_t *)"Temp:",6);
						OLED_ShowNum(50,6,data[2],2,2);

						Onet_flag= 0;
					}

					//存储旧数据
					Hum = data[0];
					Temp = data[2];
					Light_nx = data[5];
					AngleX_nx= data[6];
					AngleY_nx= data[7];

				}

					osThreadResume(DHT11TaskHandle);
					osThreadResume(LightTaskHandle);
					osThreadResume(MPU6050TaskHandle);
				}
		}

//		size_t free_heap = xPortGetFreeHeapSize();
//		printf("剩余堆：%u bytes\r\n", free_heap);

		time_out = 0;
		osDelay(250);
	}
}


/* USER CODE END Application */

