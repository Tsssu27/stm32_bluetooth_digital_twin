/**
  * @file    usart.h
  * @brief   UART Driver for UART1 (HC-05/ZS-040)
  */
#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

/**
 * @brief Initialize USART1 at 9600 baud rate on PA9 (TX) and PA10 (RX)
 */
void USART1_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */
