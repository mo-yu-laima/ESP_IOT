/*
*********************************************************************************************************
*
*	模块名称 : w25q64驱动文件
*	文件名称 : w25q64.h
*
*********************************************************************************************************
*/
#ifndef __W25Q64_H
#define __W25Q64_H

#include "main.h"


#define CS_ENABLE    HAL_GPIO_WritePin(Flash_CS_GPIO_Port,Flash_CS_Pin,GPIO_PIN_RESET)
#define CS_DISABLE   HAL_GPIO_WritePin(Flash_CS_GPIO_Port,Flash_CS_Pin,GPIO_PIN_SET)


void W25Q64_Busy(void);
void W25Q64_WriteEnable(void);
void W25Q64_SectorErase(uint32_t SectorNum);
void W25Q64_WritePage(uint8_t* date, uint32_t PageNum);
void W25Q64_ReadData(uint8_t* rdate, uint32_t addr, uint32_t len);
uint32_t Flash_FindWriteAddr(uint32_t start_addr, uint32_t max_len);
void Flash_Append(char* data);
void Flash_ReadAll(void);



#endif
