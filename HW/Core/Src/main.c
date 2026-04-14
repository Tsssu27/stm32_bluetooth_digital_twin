/**
  * @file    main.c
  * @brief   STM32F103C8T6 主程序
  *          HW-040 旋转编码器 → SG90 舵机 + OLED 显示
  *
  * ┌─────────────────────────────────────────────┐
  * │          系统架构概览                          │
  * ├──────────────┬──────────────────────────────┤
  * │  模块         │ 描述                          │
  * ├──────────────┼──────────────────────────────┤
  * │ encoder      │ PB0(CLK) EXTI0 下降沿中断     │
  * │              │ PB1(DT)  GPIO 输入判方向       │
  * │              │ PB5(SW)  EXTI9_5 复位 90°     │
  * ├──────────────┼──────────────────────────────┤
  * │ servo        │ PA0 TIM2_CH1 50Hz PWM         │
  * │              │ 角度0-180 → CCR 500-2500       │
  * ├──────────────┼──────────────────────────────┤
  * │ oled         │ PB6/PB7 I2C1 400KHz           │
  * │              │ 行0: "Pos: XXX deg"            │
  * │              │ 行2: [========   ] 进度条       │
  * └──────────────┴──────────────────────────────┘
  *
  * 主循环逻辑（轮询驱动）：
  *   1. 读取编码器当前角度（原子读）
  *   2. 若角度变化 → 更新舵机 + 刷新 OLED
  *   3. 轮询间隔：20ms（50Hz，与伺服周期同步）
  */

#include "main.h"
#include "encoder.h"
#include "servo.h"
#include "oled.h"
#include "usart.h"
#include <stdio.h>

/* ==================== 私有变量 =========================================== */
/** 上次角度缓存，用于变化检测（避免无谓刷新） */
static uint16_t g_last_angle = 0xFFFFU;  /* init to invalid */

/* ==================== 私有函数声明 ======================================= */
static void SystemClock_Config(void);
static void Error_Handler(void);

/* ==================== main =============================================== */

int main(void)
{
    /* ── 1. HAL 初始化（配置 SysTick，重置外设） ── */
    HAL_Init();

    /* ── 2. 系统时钟配置（HSE → PLL → 72 MHz） ── */
    SystemClock_Config();

    /* ── 3. 外设模块初始化 ── */
    Encoder_Init();   /* PB0/PB1/PB5 GPIO + EXTI */
    Servo_Init();     /* TIM2 CH1 PWM + PA0 AF    */
    OLED_Init();      /* I2C1 + SSD1306 初始化    */
    USART1_Init();    /* USART1 9600 TX/RX        */

    /* ── 4. 设定初始角度 90° ── */
    Encoder_SetAngle(90U);
    Servo_SetAngle(90U);
    OLED_ShowAngle(90U);
    g_last_angle = 90U;

    /* ── 5. 主循环 ── */
    while (1)
    {
        /* 原子读取当前角度（可能被 EXTI 中断修改） */
        uint16_t current_angle = Encoder_GetAngle();

        /* 仅在角度发生变化时才驱动输出，避免浪费总线带宽 */
        if (current_angle != g_last_angle)
        {
            /* 5a. 更新舵机 PWM */
            Servo_SetAngle(current_angle);

            /* 5b. 刷新 OLED 显示 */
            OLED_ShowAngle(current_angle);

            /* 5c. 串口打印同步输出 */
            printf("ANGLE:%d\n", current_angle);

            /* 5d. 记录本轮角度 */
            g_last_angle = current_angle;
        }

        /* 20ms 轮询周期（与 SG90 PWM 更新率对齐） */
        HAL_Delay(20U);
    }
    /* 永不到达此处 */
    return 0;
}

/* ==================== 时钟配置 =========================================== */

/**
  * @brief  配置系统时钟为 72 MHz
  *         HSE(8MHz) → PLL×9 → SYSCLK=72MHz
  *         APB1 = 36MHz, APB2 = 72MHz
  *
  * @note   HAL_RCC_OscConfig / HAL_RCC_ClockConfig 已在 system_stm32f1xx.c
  *         的 SystemInit 中完成底层寄存器配置。此处使用 HAL API 再确认，
  *         确保 HAL 内部的时钟变量保持同步。
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* 配置 HSE + PLL */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState            = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue      = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL         = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* 配置总线分频 */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK
                                     | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1
                                     | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;    /* HCLK  = 72MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;      /* PCLK1 = 36MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;      /* PCLK2 = 72MHz */
    /* Flash 2 等待周期（72MHz 需要） */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ==================== 错误处理 =========================================== */

/**
  * @brief  致命错误处理
  *         LED（PC13）快速闪烁，方便现场诊断
  */
static void Error_Handler(void)
{
    __disable_irq();

    /* 使能 GPIOC 时钟，点亮板载 LED (PC13 低电平有效) */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = GPIO_PIN_13;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);

    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        /* 粗延时约 100ms（不依赖 SysTick） */
        for (volatile uint32_t i = 0; i < 720000UL; i++);
    }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* 发生断言错误，触发 Error_Handler */
    (void)file;
    (void)line;
    Error_Handler();
}
#endif /* USE_FULL_ASSERT */
