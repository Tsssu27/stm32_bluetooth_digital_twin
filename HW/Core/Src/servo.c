/**
  * @file    servo.c
  * @brief   SG90 舵机驱动实现
  *
  * 时钟树说明：
  *   SYSCLK = 72 MHz
  *   APB1   = 36 MHz  (TIM2 挂在 APB1)
  *   APB1 timer clock = APB1 × 2 = 72 MHz  (当 APB 分频≠1 时，TIM 时钟 ×2)
  *
  *   PSC = 71  → TIM2 计数时钟 = 72 MHz / (71+1) = 1 MHz
  *   ARR = 19999 → 周期 = 20 000 us = 20 ms → 50 Hz ✓
  *
  * CCR 计算（整数运算，避免浮点）：
  *   CCR = SERVO_CCR_MIN + (angle * (SERVO_CCR_MAX - SERVO_CCR_MIN)) / 180
  *       = 500            + (angle * 2000) / 180
  */

#include "servo.h"

/* ==================== 私有变量 =========================================== */

/** HAL TIM 句柄 */
static TIM_HandleTypeDef htim2;

/** 当前角度缓存 */
static uint16_t g_servo_angle = 90U;

/* ==================== 私有辅助 =========================================== */

/**
  * @brief  将角度转换为 TIM2 CCR 值（无浮点）
  */
static uint32_t angle_to_ccr(uint16_t angle)
{
    if (angle > 180U) angle = 180U;
    return (uint32_t)SERVO_CCR_MIN
           + ((uint32_t)angle * (uint32_t)(SERVO_CCR_MAX - SERVO_CCR_MIN)) / 180U;
}

/* ==================== 初始化 ============================================= */

void Servo_Init(void)
{
    GPIO_InitTypeDef         GPIO_InitStruct = {0};
    TIM_OC_InitTypeDef       sConfigOC       = {0};
    TIM_ClockConfigTypeDef   sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef  sMasterConfig   = {0};

    /* ── 1. 使能外设时钟 ── */
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* ── 2. PA0 → TIM2_CH1 复用推挽输出 ── */
    GPIO_InitStruct.Pin   = GPIO_PIN_0;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;     /* 复用推挽 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;/* PWM建议使用高速 */
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ── 3. 配置 TIM2 基础参数 ── */
    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = SERVO_PSC;          /* 71 */
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = SERVO_ARR;           /* 19999 */
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim2);

    /* ── 4. 时钟源为内部时钟（默认） ── */
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

    /* ── 5. 主从关系（独立使用，无需触发输出） ── */
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

    /* ── 6. 配置 CH1 为 PWM Mode 1 ── */
    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = angle_to_ccr(90U);  /* 初始 90° */
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);

    /* ── 7. 启动 PWM 输出 ── */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    g_servo_angle = 90U;
}

/* ==================== 公共 API 实现 ====================================== */

void Servo_SetAngle(uint16_t angle)
{
    if (angle > 180U) angle = 180U;
    g_servo_angle = angle;

    /* 更新 CCR（直接写寄存器，避免每次重配置开销） */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, angle_to_ccr(angle));
}

uint16_t Servo_GetAngle(void)
{
    return g_servo_angle;
}
