/**
  * @file    encoder.h
  * @brief   HW-040 旋转编码器驱动接口
  *
  * 硬件连接：
  *   CLK (A相) → PB0  EXTI0，下降沿触发
  *   DT  (B相) → PB1  GPIO 输入（判断方向）
  *   SW  (按键)→ PB5  EXTI9_5，下降沿触发（复位角度→90°）
  *
  * 判向逻辑：
  *   CLK 下降沿触发 EXTI0，此时采样 DT：
  *     DT = 1  → 顺时针  → angle += 5
  *     DT = 0  → 逆时针  → angle -= 5
  *   按键短按 → angle = ANGLE_DEFAULT
  */

#ifndef __ENCODER_H
#define __ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ==================== 宏定义 ============================================= */
/** 编码器每步增量（度） */
#define ENCODER_STEP       5U
/** 角度最小值 */
#define ANGLE_MIN          0U
/** 角度最大值 */
#define ANGLE_MAX          180U
/** 复位默认角度 */
#define ANGLE_DEFAULT      90U

/* CLK 引脚：PB0 */
#define ENCODER_CLK_PORT   GPIOB
#define ENCODER_CLK_PIN    GPIO_PIN_0

/* DT 引脚：PB1 */
#define ENCODER_DT_PORT    GPIOB
#define ENCODER_DT_PIN     GPIO_PIN_1

/* SW 引脚：PB5 */
#define ENCODER_SW_PORT    GPIOB
#define ENCODER_SW_PIN     GPIO_PIN_5

/* ==================== 公共 API =========================================== */

/**
  * @brief  初始化编码器 GPIO 和 EXTI
  *         必须在 HAL_Init() 和 SystemClock_Config() 之后调用。
  */
void Encoder_Init(void);

/**
  * @brief  获取当前角度（线程安全读取，关中断保护）
  * @return 0 ~ 180 之间的角度值
  */
uint16_t Encoder_GetAngle(void);

/**
  * @brief  强制设置角度（可用于测试/上位机同步）
  * @param  angle  目标角度，自动限幅到 [0,180]
  */
void Encoder_SetAngle(uint16_t angle);

/**
  * @brief  EXTI0 中断服务回调（CLK 下降沿），在 stm32f1xx_it.c 中调用
  */
void Encoder_EXTI0_Callback(void);

/**
  * @brief  EXTI9_5 中断服务回调（SW 按键），在 stm32f1xx_it.c 中调用
  */
void Encoder_EXTI5_Callback(void);

#ifdef __cplusplus
}
#endif

#endif /* __ENCODER_H */
