/**
  * @file    encoder.c
  * @brief   HW-040 旋转编码器驱动实现
  *
  * 关键时序说明（HW-040 光栅编码器）：
  * ─────────────────────────────────────
  *        ┌───     CLK (A相)
  *  ──────┘    ←─ EXTI0 触发（下降沿）
  *  ─────────      DT=1 → 顺时针（angle+）
  *  ──────┐        DT=0 → 逆时针（angle-）
  *        └───
  *
  * 防抖策略：
  *   使用软件去抖延时 (1ms) 后再采样 DT，
  *   避免编码器机械抖动导致误计数。
  *
  * 按键防抖：
  *   采集到 SW 低电平后延时 20ms 再确认。
  */

#include "encoder.h"

/* ==================== 私有变量 =========================================== */

/** 当前角度（受中断保护，使用 volatile） */
static volatile uint16_t g_angle = ANGLE_DEFAULT;

/** 按键去抖时间戳 */
static volatile uint32_t g_sw_last_tick = 0U;

/* 按键去抖时间窗口 (ms) */
#define SW_DEBOUNCE_MS     200U
/* CLK 去抖循环次数（@72MHz，约 1ms）*/
#define CLK_DEBOUNCE_LOOPS 7200U
/* SW 确认延时循环次数（@72MHz，约 20ms）*/
#define SW_CONFIRM_LOOPS   144000U

/* ==================== 私有辅助 =========================================== */

/**
  * @brief  在中断上下文内使用的纯循环软件延时
  * @note   HAL_Delay 依赖 SysTick，在 EXTI 中断中会死锁，不可用。
  *         此函数不依赖任何中断。
  * @param  loops  循环次数
  */
static void delay_loops(volatile uint32_t loops)
{
    while (loops--) { __NOP(); }
}

/**
  * @brief  安全地更新角度，限幅在 [ANGLE_MIN, ANGLE_MAX]
  * @param  new_angle  新角度（有符号，允许负数输入做下溢检测）
  */
static void angle_update(int32_t new_angle)
{
    if (new_angle < (int32_t)ANGLE_MIN)
        new_angle = (int32_t)ANGLE_MIN;
    else if (new_angle > (int32_t)ANGLE_MAX)
        new_angle = (int32_t)ANGLE_MAX;

    g_angle = (uint16_t)new_angle;
}

/* ==================== 初始化 ============================================= */

void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能 GPIOB 时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* ── PB0 (CLK): 浮空输入，EXTI0 下降沿 ── */
    GPIO_InitStruct.Pin   = ENCODER_CLK_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(ENCODER_CLK_PORT, &GPIO_InitStruct);

    /* ── PB1 (DT): 普通输入，读电平判方向 ── */
    GPIO_InitStruct.Pin   = ENCODER_DT_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(ENCODER_DT_PORT, &GPIO_InitStruct);

    /* ── PB5 (SW): 浮空输入，EXTI9_5 下降沿 ── */
    GPIO_InitStruct.Pin   = ENCODER_SW_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(ENCODER_SW_PORT, &GPIO_InitStruct);

    /* 配置 EXTI0 (CLK): 优先级 1-1 */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    /* 配置 EXTI9_5 (SW): 优先级 2-0（按键低于编码器） */
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* ==================== 公共 API 实现 ====================================== */

uint16_t Encoder_GetAngle(void)
{
    uint16_t val;
    /* 关闭局部中断，原子读取 */
    __disable_irq();
    val = g_angle;
    __enable_irq();
    return val;
}

void Encoder_SetAngle(uint16_t angle)
{
    __disable_irq();
    angle_update((int32_t)angle);
    __enable_irq();
}

/* ==================== 中断回调 ============================================ */

/**
  * @brief  CLK 下降沿 → EXTI0 回调
  *         判向：读 DT 电平
  *           DT=1 → 顺时针 → angle += ENCODER_STEP
  *           DT=0 → 逆时针 → angle -= ENCODER_STEP
  */
void Encoder_EXTI0_Callback(void)
{
    /* 软件去抖：纯循环延时（中断内不可用 HAL_Delay）*/
    delay_loops(CLK_DEBOUNCE_LOOPS);

    /* 再次检测 CLK 是否仍然为低（确认是真实下降沿） */
    if (HAL_GPIO_ReadPin(ENCODER_CLK_PORT, ENCODER_CLK_PIN) != GPIO_PIN_RESET)
        return;

    /* 读取 DT 电平判断方向 */
    GPIO_PinState dt_state = HAL_GPIO_ReadPin(ENCODER_DT_PORT, ENCODER_DT_PIN);

    if (dt_state == GPIO_PIN_SET)
    {
        /* DT 高 → CLK 先下降 → 顺时针 → 增大角度 */
        angle_update((int32_t)g_angle + (int32_t)ENCODER_STEP);
    }
    else
    {
        /* DT 低 → DT 先下降 → 逆时针 → 减小角度 */
        angle_update((int32_t)g_angle - (int32_t)ENCODER_STEP);
    }
}

/**
  * @brief  SW 按键 → EXTI9_5 回调
  *         按键按下后角度复位至 ANGLE_DEFAULT (90°)
  *         带 200ms 去抖窗口防止重复触发
  */
void Encoder_EXTI5_Callback(void)
{
    uint32_t now = HAL_GetTick();

    /* 去抖窗口内的重复触发一律忽略 */
    if ((now - g_sw_last_tick) < SW_DEBOUNCE_MS)
        return;

    g_sw_last_tick = now;

    /* 延时确认按键仍被按下（纯循环，不依赖 SysTick）*/
    delay_loops(SW_CONFIRM_LOOPS);
    if (HAL_GPIO_ReadPin(ENCODER_SW_PORT, ENCODER_SW_PIN) != GPIO_PIN_RESET)
        return;

    /* 复位角度到 90° */
    angle_update((int32_t)ANGLE_DEFAULT);
}
