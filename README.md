# ESP_IOT - STM32 物联网综合实验项目

## 📋 项目概述

基于 **STM32F103C8T6** 主控芯片的物联网综合实验平台，集成 **FreeRTOS** 实时操作系统，通过 **ESP8266 WiFi 模块** 实现云端通信，搭载多种传感器与外设模块，适用于嵌入式 IoT 开发学习和原型验证。

## 🧩 硬件平台

### 主控
- **MCU**: STM32F103C8T6 (ARM Cortex-M3, 72MHz)
- **存储**: 64KB Flash, 20KB SRAM
- **RTOS**: FreeRTOS (CMSIS-RTOS V2 封装)

### 外设资源配置

| 外设 | 功能 | 接口 |
|------|------|------|
| USART1 | ESP8266 通信 / 调试串口 | PA9(TX), PA10(RX) |
| USART2 | GPS 模块通信 | PA2(TX), PA3(RX) |
| USART3 | 预留扩展串口 | PB10(TX), PB11(RX) |
| I2C1 | MPU6050 / OLED 通信 | PB6(SCL), PB7(SDA) |
| SPI1 | 板载 Flash (W25Qxx) | PA5(SCK), PA6(MISO), PA7(MOSI) |
| ADC1 | 光照传感器 (CH0) | PA0 |
| TIM1 | PWM / 定时控制 | - |

### 引脚定义

| 引脚 | 功能 | 说明 |
|------|------|------|
| PC13 | LED | 板载指示灯（低电平点亮） |
| PB12 | Flash_CS | SPI Flash 片选 |
| PB3 | A | EC11 编码器 A 相（下降沿中断） |
| PB0 | B | EC11 编码器 B 相（上拉输入） |
| PA5-PA7 | SPI1 | Flash 通信总线 |

## 📦 项目结构

`
ESP_IOT/
├── Core/                   # 核心代码
│   ├── Inc/                # 头文件 (main.h, gpio.h, usart.h 等)
│   ├── Src/                # 源文件 (main.c, freertos.c 等)
│   └── Startup/            # 启动文件 (startup_stm32f103c8tx.s)
├── Drivers/                # HAL 驱动库
│   ├── CMSIS/              # CMSIS 核心层
│   └── STM32F1xx_HAL_Driver/  # STM32F1 HAL 外设驱动
├── Middlewares/             # 中间件
│   └── Third_Party/FreeRTOS/  # FreeRTOS 源码
├── My_code/                # **用户自定义驱动模块**
│   ├── DHT11/              # 温湿度传感器
│   ├── EC11/               # 旋转编码器
│   ├── ESP/                # ESP8266 WiFi 模块
│   ├── Flash/              # SPI Flash 读写
│   ├── GPS/                # GPS 定位模块
│   ├── Light/              # 光照传感器
│   ├── MPU6050_max/        # 六轴陀螺仪+加速度计
│   ├── OLED/               # OLED 显示驱动（含字库）
│   ├── RC522/              # RFID 射频模块
│   └── uart/               # 串口通信工具
├── Debug/                  # 编译输出（忽略）
├── .settings/              # IDE 配置
├── ESP_IOT.ioc             # STM32CubeMX 项目文件
└── STM32F103C8TX_FLASH.ld  # 链接脚本
`

## 🔧 开发环境

- **IDE**: STM32CubeIDE
- **工具链**: ARM GCC
- **HAL 库**: STM32Cube FW_F1 V1.8.6
- **配置工具**: STM32CubeMX 6.15.0
- **编程语言**: C (C11)

## 🚀 快速开始

### 1. 克隆仓库
\\\ash
git clone https://github.com/YOUR_USERNAME/ESP_IOT.git
\\\

### 2. 使用 STM32CubeIDE 打开
- 打开 STM32CubeIDE
- 选择 \File → Import → Existing Projects into Workspace\
- 选择项目根目录 \ESP_IOT\

### 3. 编译与烧录
- 在 IDE 中点击 \Build\ (或使用 \make\)
- 通过 ST-Link 下载至开发板

> **注意**: 若需修改外设配置，请使用 STM32CubeMX 打开 \ESP_IOT.ioc\ 文件重新生成代码。

## 📡 模块说明

### My_code 用户驱动模块

| 模块 | 文件 | 功能描述 |
|------|------|----------|
| DHT11 | \DHT11.c/.h\ | 单总线温湿度传感器读取 |
| EC11 | \EC11.c/.h\ | 旋转编码器脉冲检测 |
| ESP | \esp8266.c/.h\ | ESP8266 AT 指令驱动，WiFi 连接与数据收发 |
| Flash | \Flash.c/.h\ | SPI Flash (W25Qxx) 读写擦除操作 |
| GPS | \GPS.c/.h\ | GPS NMEA 协议解析与定位数据提取 |
| Light | \Light.c/.h\ | ADC 光照强度采集与转换 |
| MPU6050_max | \MPU6050_max.c/.h\ | 六轴姿态传感器初始化与数据读取 |
| OLED | \OLED.c/.h\ + \OLEDfont.h\ | 0.96 寸 OLED 显示驱动（含中文字库） |
| RC522 | \RC522.c/.h\ | RFID 射频卡读写（SPI 接口） |
| uart | \uart.c/.h\ | 串口 DMA 收发与数据帧解析 |

### 核心功能流程

系统上电 → HAL 初始化 → 外设初始化 (MX_xxx_Init) → 开启串口中断接收 → BSP_Init() 初始化各模块 → OLED 显示启动信息 → ESP8266 连接 WiFi → FreeRTOS 内核启动 (osKernelStart) → 任务调度运行

## 📜 许可

本项目仅用于学习和实验目的。STM32 HAL 库和 FreeRTOS 部分分别遵循其各自的开源许可协议。
