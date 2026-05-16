#include "EC11.h"
#include "oled.h"

uint8_t EC11_flag= 0;
uint8_t idex= 1;

uint8_t Onet_flag= 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)  //沿触发
{
	if(GPIO_Pin == A_Pin)
	{

		if(HAL_GPIO_ReadPin(B_GPIO_Port, B_Pin))  //顺时针旋转
		{
			OLED_Clear();

			if(EC11_flag== 3)	//依旧获取完报警信息了，此时旋转EC11会退回到温湿度显示页面
			{
				idex= 1;

				EC11_flag= 0;	//初始化
				OLED_Clear();
				Onet_flag= 1;	//允许显示温湿度页面
			}
			else
			{
				EC11_flag= 1;
				idex++;
			}
		}
		else	//逆时针旋转
		{
			EC11_flag= 2;
			idex= 1;
		}
	}
}
