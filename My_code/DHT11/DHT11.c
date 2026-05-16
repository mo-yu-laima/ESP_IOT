#include "main.h"
#include "tim.h"
#include "dht11.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include "esp8266.h"
#include "Flash.h"
#include "oled.h"

extern TimeData time_data;
extern osTimerId_t UTCHandle;

extern char json[200];
extern char cmd[300];

extern SensorData_t tx_buff;
extern uint8_t Task_flag;
extern uint8_t month;
extern uint8_t EC11_flag;

extern osMessageQueueId_t myQueue01Handle;


uint8_t Data[5]={0x00,0x00,0x00,0x00,0x00};   //Data存储读取的温湿度信息

void Delay_us(uint16_t us)
{     //微秒延时
	uint16_t differ = 0xffff-us-5;
	__HAL_TIM_SET_COUNTER(&htim1,differ);	//设定TIM1计数器起始值
	HAL_TIM_Base_Start(&htim1);		//启动定时器

	while(differ < 0xffff-5){	//判断
		differ = __HAL_TIM_GET_COUNTER(&htim1);		//查询计数器的计数值
	}
	HAL_TIM_Base_Stop(&htim1);
}

/*------------------------------*/
void DHT_GPIO_SET_OUTPUT(void)     //设置GPIOx为输出模式（MCU的IO口向DHT11发激活信号）
{
	GPIO_InitTypeDef GPIO_InitStructure;    //在GPIO_InitTypeDef结构体中修改IO口参数（结构体成员）
	GPIO_InitStructure.Pin=DHT11_data_Pin;      //设置的格式必须严格遵循注释，比如GPIO_PIN_define
	GPIO_InitStructure.Mode=GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStructure.Pull=;
	GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DHT11_data_GPIO_Port,&GPIO_InitStructure);
}

void DHT_GPIO_SET_INPUT(void)     //设置GPIOx为输入模式（DHT11向MUC的IO发电平信号，信号里包含了温湿度信息）
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin=DHT11_data_Pin;
	GPIO_InitStructure.Mode=GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed=GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(DHT11_data_GPIO_Port,&GPIO_InitStructure);
}

/*------------------------------*/

/*
     uint8_t DHT_Read_Byte(void)用于转译采集DHT11发送给IO口的电平信号（8位）。
*/
uint8_t DHT_Read_Byte(void)  //从DHT11读取一位（8字节）信号
{
	 uint8_t ReadData=0;  //ReadData用于存放8bit数据，即8个单次读取的1bit数据的组合
	 uint8_t temp;      //临时存放信号电平（0或1）
	 uint8_t retry=0;   //retry用于防止卡死
	 uint8_t i;
	 for(i=0; i<8; i++)   //一次温湿度信号读取八位
	 {
			while(HAL_GPIO_ReadPin(DHT11_data_GPIO_Port,DHT11_data_Pin)==0 && retry<100)
			//等待直到DHT11输出高电平：当PA5=1，上升沿，表示开始接受数据，可以判断0 or 1，跳出循环，执行后续判断（若PA5=0，将一直循环等待）
			{
					Delay_us(1);
				  retry++;             //retry防止PA5读取不到数据卡死在这一步，当经历100us后retry自增到100，跳出循环。
			}
			retry=0;

			Delay_us(40);    //延时30us
			//根据时序图，DHT传回高电平信号维持26us~28us表示0，	维持70us表示1
		  //延时30us后，如果IO读取到仍是高电平，说明采集到1；如果IO读取到低电平，说明采集到0
			//读取电平信号暂存temp内，随后会压入ReadData中
			if(HAL_GPIO_ReadPin(DHT11_data_GPIO_Port,DHT11_data_Pin)==1)   temp=1;
			 else   temp=0;

			 while(HAL_GPIO_ReadPin(DHT11_data_GPIO_Port,DHT11_data_Pin)==1 && retry<100)
			//等待直到DHT11输出低电平，表示退出。本轮1bit信号接收完毕。
			 {
				 Delay_us(1);
				 retry++;
			 }
			 retry=0;

			 ReadData<<=1;    //ReadData内信号先全部左移一位，空出末尾位置
			 ReadData |= temp;        //将temp写入ReadData
	 }

		return ReadData;
}

/*------------------------------*/

/*
     uint8_t DHT_Read(void)表达完整时序
*/
uint8_t DHT_Read(void)
{
	 uint8_t retry=0;
	 uint8_t i;

	 DHT_GPIO_SET_OUTPUT();    //IO设置为输出模式。在传输的最开始，MCU要向DHT11发送信号
	 HAL_GPIO_WritePin(DHT11_data_GPIO_Port,DHT11_data_Pin,GPIO_PIN_RESET);   //IO->DHT11:先拉低电平18ms（应时序要求）
	 HAL_Delay(18);
	 HAL_GPIO_WritePin(DHT11_data_GPIO_Port,DHT11_data_Pin,GPIO_PIN_SET);   //IO->DHT11:随后拉高电平20us
	 Delay_us(20);
	
	//MCU通过IO向DHT11发送请求完毕。接下来DHT11向IO发送响应,IO转为输入模式。在这之后就开始信号的转译读取。
	DHT_GPIO_SET_INPUT();
	Delay_us(20);
	if(HAL_GPIO_ReadPin(DHT11_data_GPIO_Port,DHT11_data_Pin)==0) //DHT11发回低电平响应（读取到低电平，说明DHT11有响应）
	{
		//接下来，DHT11拉低电平一段时间后拉高电平一段时间
		while(HAL_GPIO_ReadPin(DHT11_data_GPIO_Port,DHT11_data_Pin)==0 && retry<100)
		{
		   Delay_us(1);
			 retry++;
		}
		retry=0;
		while(HAL_GPIO_ReadPin(DHT11_data_GPIO_Port,DHT11_data_Pin)==1 && retry<100)
		{
		   Delay_us(1);
			 retry++;
		}
		retry=0;

		//一共传输40位，一次DHT_Read_Byte返回8位，共读取5次。存储在Data[]中。（Data[]定义为全局）
		for(i=0; i<5; i++)
		{
			 tx_buff.Data[i] = DHT_Read_Byte();  //每次读取一字节（8位）
		}
		Delay_us(50);
		//说明：Data[0]湿度， Data[2]温度。Data[1]和Data[3]分别为0和2的小数位。Data[4]用于校验。
	}

	 uint32_t sum=tx_buff.Data[0]+tx_buff.Data[1]+tx_buff.Data[2]+tx_buff.Data[3];  //校验
	 if((sum)==tx_buff.Data[4])    return 1;
	   else   return 0;

}



/*********************************************************************************/
uint8_t H_flag= 0, T_flag= 0;
uint8_t H_second= 0, T_second= 0;

//同Light.c内的存储报警信息
void Hum_flash()
{
	if((tx_buff.Data[0] >=65 || tx_buff.Data[0] <= 30) && tx_buff.Data[0] != 99 && tx_buff.Data[0]!= 1 && H_flag== 0 && EC11_flag== 0)
	{
		char logbuf[64];
		if(tx_buff.Data[0] >=65)
		{
			sprintf(logbuf,
					"%d %02d %d %02d:%02d:%02d HH\n",
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
					"%d %02d %d %02d:%02d:%02d HL\n",
					time_data.year,
					month,
					time_data.day,
					time_data.hour,
					time_data.minute,
					time_data.second
					);
		}

		H_second= time_data.second;

		Flash_Append(logbuf);
//				Flash_ReadAll();

		H_flag= 1;
	}
	else if (H_flag == 1)
	{
	    uint8_t diff = (time_data.second + 60 - H_second) % 60;
	    if (diff >= 10)
	        H_flag = 0;
	}

}


void Temp_flash()
{
	if((tx_buff.Data[2] >=35 || tx_buff.Data[2] <= 10) && tx_buff.Data[2] != 99 && tx_buff.Data[2]!= 1 && T_flag== 0 && EC11_flag== 0)
	{
		char logbuf[64];
		if(tx_buff.Data[2] >=35)
		{
			sprintf(logbuf,
					"%d %02d %d %02d:%02d:%02d TH\n",
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
					"%d %02d %d %02d:%02d:%02d TL\n",
					time_data.year,
					month,
					time_data.day,
					time_data.hour,
					time_data.minute,
					time_data.second
					);
		}

		T_second= time_data.second;

		Flash_Append(logbuf);
//				Flash_ReadAll();

		T_flag= 1;
	}
	else if (T_flag == 1)
	{
	    uint8_t diff = (time_data.second + 60 - T_second) % 60;
	    if (diff >= 10)
	        T_flag = 0;
	}
}



void DHT11Task(void)
{
//	osThreadSuspend(NULL);
	do
	{
		osDelay(2000);
	}while(!Task_flag);

	uint8_t Hum= 0, Temp= 0;

	while(1)
	{
		if(DHT_Read())
		{
			//刚刚开机，数据不稳定
			if(tx_buff.Data[0] == 0) tx_buff.Data[0] = 1;
			if(tx_buff.Data[2] == 0) tx_buff.Data[2] = 1;
			if(tx_buff.Data[0] >= 100) tx_buff.Data[0] = 99;
			if(tx_buff.Data[2] >= 100) tx_buff.Data[2] = 99;

			if(tx_buff.Data[0]>= Hum+3 || tx_buff.Data[0]<= Hum-3)	//湿度容易变化，避免短时间内频繁的湿度跳变
			{
				osMessageQueuePut (myQueue01Handle, &tx_buff, 0, 0);
				Hum= tx_buff.Data[0];
			}

			Hum_flash();
			Temp_flash();

//			printf("\nHum:%d %%\n",tx_buff.Data[0]);
//			printf("Temp:%d C\n",tx_buff.Data[2]);
//			OLED_ShowString(0,2,(uint8_t *)tx_buff.Data[0],strlen(tx_buff.Data[0]));
//			OLED_ShowString(0,4,(uint8_t *)tx_buff.Data[2],strlen(tx_buff.Data[2]));

		}
		osDelay(2000);
	}


}
