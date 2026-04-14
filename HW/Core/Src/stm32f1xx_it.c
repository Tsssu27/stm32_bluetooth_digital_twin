/**
  * @file    stm32f1xx_it.c
  * @brief   STM32F1xx 中断服务程序
  *
  * 架构说明：
  *   本文件只负责 IRQ → 模块回调的"路由"，不包含任何业务逻辑。
  *   业务逻辑完全封装在各模块的 Callback 函数中。
  *
  * 关联的 EXTI：
  *   EXTI0     ← PB0 (CLK)  → Encoder_EXTI0_Callback()
  *   EXTI9_5   ← PB5 (SW)   → Encoder_EXTI5_Callback()
  */

#include "stm32f1xx_it.h"
#include "stm32f1xx_hal.h"
#include "encoder.h"
#include "usart.h"

/* ==================== 系统异常处理 ======================================= */

/**
  * @brief  NMI 异常
  */
void NMI_Handler(void)
{
    while (1) {}
}

/**
  * @brief  HardFault 异常 — 捕获并挂起，方便调试器分析
  */
void HardFault_Handler(void)
{
    /* 建议连接调试器后在此处设断点，查看调用栈 */
    while (1) {}
}

/**
  * @brief  SysTick 中断 — HAL 时基
  *         HAL_Delay / HAL_GetTick 依赖于此
  */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/* ==================== 外部中断处理 ======================================= */

/**
  * @brief  EXTI0 (PB0 = CLK 下降沿) → 旋转编码器计数
  *
  * 执行流程：
  *   1. HAL_GPIO_EXTI_IRQHandler 清除 EXTI pending flag
  *   2. 调用编码器方向判断回调
  */
void EXTI0_IRQHandler(void)
{
    /* HAL 函数负责清除 PR 寄存器中的挂起标志 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);

    /* 编码器方向判断 + 角度更新 */
    Encoder_EXTI0_Callback();
}

/**
  * @brief  EXTI9_5 (PB5 = SW 下降沿) → 编码器按键复位
  *
  * 注意：EXTI9_5 是共享 IRQ，需要在回调中判断来源引脚。
  *       本工程仅挂载 PB5，HAL 会自动管理 pending flag。
  */
void EXTI9_5_IRQHandler(void)
{
    /* 清除 PB5 的 EXTI pending flag */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);

    /* 只有 PB5 触发时才复位角度 */
    Encoder_EXTI5_Callback();
}

/**
  * @brief  This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}
