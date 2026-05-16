#include "Light.h"
#include "cmsis_os.h"
#include <stdio.h>
#include "esp8266.h"
#include "oled.h"
#include "Flash.h"

volatile uint16_t adc_value = 0;
uint8_t Vin = 0 ;

extern SensorData_t tx_buff;
extern uint8_t Task_flag;
extern uint8_t EC11_flag;
extern TimeData time_data;
extern uint8_t month;

uint8_t Light;
uint8_t L_flag= 0;	//是否运行上传报警信息给flash的标志位
uint8_t no_flag= 0;	//开机时不允许存储报警信息的标志位



extern osMessageQueueId_t myQueue01Handle;

void Get_Light_AO()
{
	HAL_ADC_Start_DMA(&hadc1, (uint16_t*)&adc_value, 1);	//DMA通道获取，解放CPU

	Vin = (adc_value/4095.0f)*3.3;		//AO口的输出电压
	tx_buff.Light = 100 - (adc_value/4095.0f)*100;  //光照强度

	HAL_ADC_Stop_DMA(&hadc1);

}

//void Get_Light_AO()
//{
//    HAL_ADC_Start(&hadc1);
//
//    HAL_ADC_PollForConversion(&hadc1 ,50);
//
//    if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
//    {
//    	adc_value = HAL_ADC_GetValue(&hadc1);
//      	Vin = (adc_value/4095)*3.3;		//AO口的输出电压
//      	Light = 100 - (adc_value/4095)*100;  //光照强度
//    }
//}

/*********************************************************************************/
uint8_t L_second= 0;	//旧时间数据

void Light_flash()	//上传报警信息
{
	if((tx_buff.Light >=70 || tx_buff.Light <= 10) && tx_buff.Light != 99 && tx_buff.Light!= 0 && L_flag== 0 && EC11_flag== 0 && no_flag== 5)
	{
		char logbuf[64];
		if(tx_buff.Light >=70)
		{
			sprintf(logbuf,
					"%d %02d %d %02d:%02d:%02d LH\n", //LH 表示Light Hight  高亮
					time_data.year,
					month,
					time_data.day,
					time_data.hour,
					time_data.minute,
					time_data.second
					);
		}
		else
		{
			sprintf(logbuf,
					"%d %02d %d %02d:%02d:%02d LL\n",
					time_data.year,
					month,
					time_data.day,
					time_data.hour,
					time_data.minute,
					time_data.second
					);
		}

		L_second= time_data.second;	//第一次存储时间

		Flash_Append(logbuf);	//存储数据到flas内
//				Flash_ReadAll();

		L_flag= 1;	//更新标志位
	}
	else if (L_flag == 1)
	{
	    uint8_t diff = (time_data.second + 60 - L_second) % 60;	//10秒后更新标志位，避免短时间内重复存储报警信息
	    if (diff >= 10)
	        L_flag = 0;
	}
}



void LightTask(void)
{
//	osThreadSuspend(NULL);
	do
	{
		osDelay(1000);
	}while(!Task_flag);	//等待时间校准

	while(1)
	{
	  Get_Light_AO();
	  if(tx_buff.Light == 255 || tx_buff.Light == 0) tx_buff.Light = 1;	//开机的时候数据不稳，需要调整一下，不然上传不了云平台

	  osMessageQueuePut (myQueue01Handle, &tx_buff, NULL, osWaitForever);	//塞数据到消息队列内


//	  printf("\nLight:%d %%",Light);

	  Light_flash();

	  no_flag++;
	  if(no_flag>= 5)	//开机时获取的数据是不稳定的，会触发报警日志存储，所以要摒弃这部分不稳定的数据
	  {
		  no_flag= 5;
	  }

	  osDelay(50);
	}
}
