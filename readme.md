# Graduation project： Design and implementation of an RTOS based on the Cortex-M3 processor
## 功能
1. 不同优先级任务按抢占式优先级算法调度，支持多任务同优先级按时间片调度
2. 支持基本的临界区保护、调度锁
3. 支持基本任务的创建、删除、延时、挂起、唤醒等
4. 支持计数信号量、互斥信号量
5. 支持基于块的内存管理机制
6. 支持事件标志组
7. 支持邮箱机制
8. 支持软定时器
9. 支持hooks函数
10. 支持低功耗模式
11. 支持内核裁剪
## 现已适配Cortex-M4内核，支持开启FPU
可在文件`rtConfig.h`中进行配置
## 分支[`Respondent_version`](https://git.acwing.com/hjw_iie/my_rtos/-/tree/Respondent_version)针对STM32F407进行了的适配，支持多个驱动与命令行功能
串口命令行建议使用[此项目](https://zhuanlan.zhihu.com/p/381099251)，即在Windows Terminal中使用串口连接，十分好用
