/**
  * @file    servo.h
  * @brief   SG90 舵机驱动接口
  *
  * 硬件连接：
  *   信号线 → PA0 (TIM2_CH1，AF 复用推挽输出)
  *
  * PWM 参数（SYSCLK = 72 MHz）：
  *   预分频 PSC = 71  → TIM2 计数频率 = 1 MHz
  *   自动重载 ARR = 19999 → 周期 = 20 ms = 50 Hz
  *
  * 角度 → CCR 映射（线性插值）：
  *   0°   → CCR =  500  (0.5 ms)
  *   90°  → CCR = 1500  (1.5 ms)
  *   180° → CCR = 2500  (2.5 ms)
  *
  *   公式：CCR = 500 + angle * (2000 / 180)
  *              ≈ 500 + angle * 11
  */

#ifndef __SERVO_H
#define __SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ==================== 宏定义 ============================================= */
/** TIM2 时钟频率（Hz） - 等于 SystemCoreClock / (PSC+1) */
#define SERVO_TIM_CLK       1000000UL   /* 1 MHz */
/** PWM 周期计数值（ARR） → 20 ms */
#define SERVO_ARR           19999U
/** 预分频（PSC），使 APB1_TIM = 72 MHz → 1 MHz */
#define SERVO_PSC           71U

/** 0° 对应的 CCR 值（0.5 ms 脉宽） */
#define SERVO_CCR_MIN       500U
/** 180° 对应的 CCR 值（2.5 ms 脉宽） */
#define SERVO_CCR_MAX       2500U

/* ==================== 公共 API =========================================== */

/**
  * @brief  初始化 TIM2_CH1，输出 50Hz PWM，舵机停在 90°
  *         必须在 HAL_Init() 和 SystemClock_Config() 之后调用。
  */
void Servo_Init(void);

/**
  * @brief  设置舵机目标角度
  * @param  angle  目标角度，范围 0 ~ 180（度）
  *                超出范围自动限幅
  */
void Servo_SetAngle(uint16_t angle);

/**
  * @brief  获取当前舵机角度（即上次 SetAngle 设定的值）
  * @return uint16_t   0 ~ 180
  */
uint16_t Servo_GetAngle(void);

#ifdef __cplusplus
}
#endif

#endif /* __SERVO_H */
