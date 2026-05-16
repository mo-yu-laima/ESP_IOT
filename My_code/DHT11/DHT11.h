#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f1xx.h"

void Delay_us(uint16_t us);
void DHT_GPIO_SET_OUTPUT(void);
void DHT_GPIO_SET_INPUT(void);
uint8_t DHT_Read_Byte(void);
uint8_t DHT_Read(void);
void DHT11Task(void);

#endif
