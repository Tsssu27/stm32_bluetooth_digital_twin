/**
  * @file    stm32f1xx_hal_conf.h
  * @brief   HAL configuration — enable only what this project uses.
  *
  * Modules enabled:
  *   - HAL_MODULE_ENABLED   (base HAL)
  *   - HAL_GPIO_MODULE_ENABLED
  *   - HAL_RCC_MODULE_ENABLED
  *   - HAL_TIM_MODULE_ENABLED   (TIM2 PWM → SG90)
  *   - HAL_I2C_MODULE_ENABLED   (I2C1  → OLED)
  *   - HAL_EXTI_MODULE_ENABLED  (EXTI0, EXTI9_5 → 编码器)
  *   - HAL_CORTEX_MODULE_ENABLED
  *   - HAL_FLASH_MODULE_ENABLED
  *   - HAL_DMA_MODULE_ENABLED
  */

#ifndef __STM32F1XX_HAL_CONF_H
#define __STM32F1XX_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ================== Module Selection ===================================== */
#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
/* (Peripherals NOT used — keep commented to save code space)
#define HAL_ADC_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_USART_MODULE_ENABLED
#define HAL_CAN_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RTC_MODULE_ENABLED
*/

/* ================== HSE / HSI values ===================================== */
#if !defined(HSE_VALUE)
  #define HSE_VALUE    8000000UL   /*!< Bluepill crystal = 8 MHz */
#endif

#if !defined(HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT  100UL   /*!< ms */
#endif

#if !defined(HSI_VALUE)
  #define HSI_VALUE    8000000UL
#endif

#if !defined(LSE_VALUE)
  #define LSE_VALUE    32768UL     /*!< LSE Typical Value in Hz */
#endif

#if !defined(LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT  5000UL /*!< Time out for LSE start up, in ms */
#endif

#if !defined(LSI_VALUE)
  #define LSI_VALUE    40000UL     /*!< LSI Typical Value in Hz */
#endif

/* ================== Systick / tick config ================================= */
#define TICK_INT_PRIORITY   0x0FUL    /* Lowest priority */
#define USE_RTOS            0U
#define PREFETCH_ENABLE     1U

/* ================== VDD / calibration ==================================== */
#define VDD_VALUE           3300UL
#define TICK_INT_PRIORITY   0x0FUL
#define VREFINT_CAL_VREF    3300UL

/* ================== Include HAL peripheral headers ======================= */
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_exti.h"
#ifdef HAL_TIM_MODULE_ENABLED
  #include "stm32f1xx_hal_tim.h"
#endif
#ifdef HAL_I2C_MODULE_ENABLED
  #include "stm32f1xx_hal_i2c.h"
#endif
#ifdef HAL_UART_MODULE_ENABLED
  #include "stm32f1xx_hal_uart.h"
#endif

/* ================== Assert macro ========================================= */
#ifdef USE_FULL_ASSERT
  #define assert_param(expr) \
      ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_CONF_H */
