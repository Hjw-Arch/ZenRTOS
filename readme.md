# Demo version
此版本为展示所用
## 说明
- 此版本适配于STM32F407 MCU，支持开启FPU，支持低功耗模式，内核可裁剪，主频为168MHZ。
- 此版本基于`master`分支中的`ZenRTOS`内核与正点原子STM32F407探索版开发板的少量板上资源，加入与`ZenRTOS`适配后的驱动。
- 此版本通过'USART1'串口通信支持了命令行, 具体功能可查看`CLI & Monitor`中的`cli.c`文件。
