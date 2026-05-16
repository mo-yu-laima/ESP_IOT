/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
// 定义传感器数据结构体
typedef struct {
    uint8_t Data[5];  // 温湿度数据（5个元素）
    uint8_t Light;    // 光照数据（单独成员，对应你要的data[5]）
    int8_t Angle[2];
} SensorData_t;

typedef struct {
    char weekday[4];
    char month[4];
    int year;
    int day;
    int hour;
    int minute;
    int second;
} TimeData;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define AO_Pin GPIO_PIN_0
#define AO_GPIO_Port GPIOA
#define DHT11_data_Pin GPIO_PIN_1
#define DHT11_data_GPIO_Port GPIOA
#define B_Pin GPIO_PIN_0
#define B_GPIO_Port GPIOB
#define A_Pin GPIO_PIN_3
#define A_GPIO_Port GPIOB
#define A_EXTI_IRQn EXTI3_IRQn
#define Flash_CS_Pin GPIO_PIN_9
#define Flash_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define LED_ON HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,0)
#define LED_OFF HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,1)
#define LED_TO HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
