/*
*********************************************************************************************************
*
*	模块名称 : w25q64驱动文件
*	文件名称 : w25q64.c
*
*********************************************************************************************************
*/
#include "Flash.h"
#include "spi.h"


/**
 * @brief	W25Q64等待空闲。读取状态寄存器，查询BUSY状态位命令。
 *
 * @return  void
 */
void W25Q64_Busy(void)
{
    uint8_t cmd[2],data[2];

    cmd[0] = 0x05;  // 读寄存器指令
    cmd[1] = 0xFF;  // 激活时钟
    
    do
    {
    	CS_ENABLE;
        HAL_SPI_TransmitReceive(&hspi1,cmd,data,2,1000);
    	CS_DISABLE;
    	
    } while ((data[1]&0x01)==0x01);     // 判断 BUSY位 是否为 0
}


/**
 * @brief	W25Q64写使能。
 *
 * @return  void
 */
void W25Q64_WriteEnable(void)
{
    uint8_t cmd;

    cmd = 0x06;
    
    W25Q64_Busy();      // 等待W25Q64空闲
    CS_ENABLE;
    HAL_SPI_Transmit(&hspi1,&cmd,1,1000);
    CS_DISABLE;
}


/**
 * @brief	擦除一个扇区  4kB
 *
 * @param   SectorNum   指定扇区编号
 *
 * @return  void
 */
void W25Q64_SectorErase(uint32_t SectorNum)
{
    uint8_t cmd[4];
    
    cmd[0] = 0x20;  // 擦除寄存器指令
    cmd[1] = (SectorNum*4096)>>16;
    cmd[2] = (SectorNum*4096)>>8;
    cmd[3] = (SectorNum*4096)>>0;
    
    W25Q64_WriteEnable();   // W25Q64写使能
    W25Q64_Busy();      // 等待W25Q64空闲
    CS_ENABLE;
    HAL_SPI_Transmit(&hspi1,cmd,4,1000);
    CS_DISABLE;
}


/**
 * @brief	W25Q64写入一个页
 *
 * @param   date      待保存数据
 * @param   PageNum   指定页编号
 *
 * @return  void
 */
void W25Q64_WritePage(uint8_t* date, uint32_t PageNum)
{
    uint8_t cmd[4];
    
    cmd[0] = 0x02;  // 擦除寄存器指令
    cmd[1] = (PageNum*256)>>16;
    cmd[2] = (PageNum*256)>>8;
    cmd[3] = (PageNum*256)>>0;
    
    W25Q64_WriteEnable();   // W25Q64写使能
    W25Q64_Busy();      // 等待W25Q64空闲
    CS_ENABLE;
    HAL_SPI_Transmit(&hspi1,cmd,4,1000);
    HAL_SPI_Transmit(&hspi1,date,256,5000);
    CS_DISABLE;
}


/**
 * @brief	W25Q64读取数据
 *
 * @param   rdate   读取数据的保存地址
 * @param   addr    读取数据的地址
 * @param   len     读取数据的长度
 *
 * @return  void
 */
void W25Q64_ReadData(uint8_t* rdate, uint32_t addr, uint32_t len)
{
    uint8_t cmd[4];
    
    cmd[0] = 0x03;  // 擦除寄存器指令
    cmd[1] = addr>>16;
    cmd[2] = addr>>8;
    cmd[3] = addr>>0;

    W25Q64_Busy();      // 等待W25Q64空闲
    CS_ENABLE;
    HAL_SPI_Transmit(&hspi1,cmd,4,1000);
    HAL_SPI_Receive(&hspi1,rdate,len,50000);
    CS_DISABLE;
}



uint32_t Flash_FindWriteAddr(uint32_t start_addr, uint32_t max_len)	//获取当前可以填写数据的地址位
{
    uint8_t buf[256];	//一页的数据
    uint32_t addr = start_addr;

    while (addr < start_addr + max_len)
    {
        W25Q64_ReadData(buf, addr, 256);

        for (int i = 0; i < 256; i++)
        {
            if (buf[i] == 0xFF)
            {
                // 找到空位
                return addr + i;
            }
        }

        addr += 256;   // 下一页
    }

    return 0xFFFFFFFF; // Flash 满了
}



void Flash_Append(char* data)	//在空位处写入数据，允许跨页写入（会自动跨页）
{
    uint32_t len = strlen(data);
    uint32_t addr = Flash_FindWriteAddr(0, 65536); // 64kb

    if (addr == 0xFFFFFFFF) return;  // Flash满

    // 写入时必须写满一页 256B（驱动强制）
    // 所以我们按页分段写
    while (len > 0)
    {
        uint8_t page_buf[256];
        for(uint8_t i= 0;i< 255;i++)
        {
        	page_buf[i]= 0xFF;
        }

        uint32_t page_offset = addr % 256;	//当前页剩余的可写空间
        uint32_t write_len = 256 - page_offset;	 //data在当前页可写入的数据长度

        if (write_len > len) write_len = len;

        memcpy(&page_buf[page_offset], data, write_len);

        uint32_t page_addr = addr - page_offset;

        W25Q64_WritePage(page_buf, page_addr / 256);

        addr += write_len;
        data += write_len;
        len  -= write_len;
    }
}


void Flash_ReadAll(void)  //读取flash内所有数据
{
    uint8_t buf;
    uint32_t addr = 0;

    while (1)
    {
        W25Q64_ReadData(&buf, addr, 1);
//        printf("ADDR:%lu  BYTE:0x%02X '%c'\r\n", addr, buf, buf);
        if (buf == 0xFF) break;
        printf("%c", buf);
        addr++;
    }
}

