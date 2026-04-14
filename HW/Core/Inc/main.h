/**
  * @file    main.h
  * @brief   主程序公共头文件
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* 错误处理弱化接口（HAL库有时会调用此宏） */
#define Error_Handler_Macro() do { while(1); } while(0)

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
