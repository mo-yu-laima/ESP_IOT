# AGENTS.md - ESP_IOT 项目编码指南

## 项目概述

本项目是基于 STM32F103C8T6 的 FreeRTOS 物联网综合实验项目，使用 STM32CubeIDE (ARM GCC) 工具链开发。

## 项目规范

### 目录结构
- \Core/Inc\ 和 \Core/Src\: STM32CubeMX 生成的核心代码
- \Drivers/\: STM32 HAL 驱动库（**请勿手动修改**）
- \Middlewares/\: FreeRTOS 源码（**请勿手动修改**）
- \My_code/\: **用户自定义驱动模块** — 所有新增功能代码均放在此目录下
- \Debug/\: 编译输出目录（不提交到 Git）

### 编码规范

1. **命名规则**
   - 文件和目录: PascalCase (如 \DHT11.c\, \MPU6050_max.c\)
   - 函数名: 驼峰命名 (如 \ESP8266_Init\)
   - 宏定义: 大写 + 下划线 (如 \OLED_CMD\)
   - 全局变量: 见名知意，避免单字母变量

2. **头文件规范**
   - 每个模块必须包含 \.c\ 和 \.h\ 文件
   - 头文件使用 \#ifndef\ / \#define\ / \#endif\ 防止重复包含
   - 在 \My_code\ 目录下的模块头文件中定义模块接口

3. **模块化开发**
   - My_code 下的每个子目录代表一个独立模块
   - 各模块之间尽量减少耦合
   - 模块初始化统一在 \BSP_Init()\ 中调用
   - 模块内部静态函数使用 \static\ 限定

4. **FreeRTOS 任务**
   - 任务创建在 \reertos.c\ 的 \MX_FREERTOS_Init\ 中
   - 任务函数命名: \Start[任务名]Task\
   - 队列/信号量命名: \[描述]Queue\, \[描述]Semaphore\

5. **STM32CubeMX 注意事项**
   - 用户代码放在 \USER CODE BEGIN\ / \USER CODE END\ 注释之间
   - 重新生成代码时，CubeMX 只会保留 BEGIN/END 之间的代码
   - 修改外设配置后必须重新生成代码

### 修改限制

- ❌ **禁止**删除或批量修改文件（遵循 AGENTS.md 的批量删除禁令）
- ❌ **禁止**修改 \Drivers/\ 和 \Middlewares/\ 目录下的文件
- ❌ **禁止**修改 CubeMX 自动生成的初始化函数（MX_xxx_Init）
- ✅ **允许**在 \My_code/\ 下添加或修改用户驱动模块
- ✅ **允许**在 \Core/Src/main.c\ 和 \Core/Src/freertos.c\ 的 USER CODE 区域添加代码

## Git 提交规范

- \Debug/\ 目录不提交
- 提交信息使用中文，清晰描述改动内容
- 不要提交编译产物 (\.o\, \.elf\, \.hex\, \.map\, \.list\)
