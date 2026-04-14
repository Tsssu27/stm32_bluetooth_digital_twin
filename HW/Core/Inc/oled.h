/**
  * @file    oled.h
  * @brief   SSD1306 0.96" I2C OLED 驱动接口（128×64，软件字库）
  *
  * 硬件连接：
  *   SCL → PB6 (I2C1_SCL)
  *   SDA → PB7 (I2C1_SDA)
  *   VCC → 3.3V
  *   GND → GND
  *
  * 功能：
  *   - 基本文字显示（5×8 像素 ASCII 字符集）
  *   - 进度条绘制
  *   - 整屏刷新
  *
  * OLED I2C 地址：
  *   SA0=0 → 0x3C (默认)
  *   SA0=1 → 0x3D
  */

#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ==================== 参数配置 =========================================== */
/** OLED I2C 7-bit 地址（左移1位后为写地址 0x78） */
#define OLED_I2C_ADDR       0x3CU

/** 屏幕宽度（像素） */
#define OLED_WIDTH          128U
/** 屏幕高度（像素） */
#define OLED_HEIGHT         64U
/** 页数（每页 8 行像素） */
#define OLED_PAGES          8U

/* I2C 超时 (ms) */
#define OLED_I2C_TIMEOUT    10U

/* ==================== 类型定义 =========================================== */

/** 显示颜色 */
typedef enum {
    OLED_COLOR_BLACK = 0,  /*!< 像素关 */
    OLED_COLOR_WHITE = 1   /*!< 像素开 */
} OLED_Color_t;

/* ==================== 公共 API =========================================== */

/**
  * @brief  初始化 I2C1 和 SSD1306
  *         调用之前须完成 HAL_Init() 和时钟配置。
  */
void OLED_Init(void);

/**
  * @brief  清除显存（全部填 0）
  */
void OLED_Clear(void);

/**
  * @brief  将显存数据刷新到 OLED 屏幕
  */
void OLED_Update(void);

/**
  * @brief  在指定坐标绘制一个字符（5×8 字体）
  * @param  x     列坐标，0 ~ 127
  * @param  page  页坐标，0 ~ 7（每页 = 8 像素高）
  * @param  ch    ASCII 字符
  * @param  color OLED_COLOR_WHITE / OLED_COLOR_BLACK
  */
void OLED_DrawChar(uint8_t x, uint8_t page, char ch, OLED_Color_t color);

/**
  * @brief  在指定位置绘制字符串
  * @param  x     列起始坐标
  * @param  page  页起始坐标
  * @param  str   以 '\0' 结尾的字符串
  * @param  color 颜色
  */
void OLED_DrawString(uint8_t x, uint8_t page, const char *str, OLED_Color_t color);

/**
  * @brief  在指定页绘制水平进度条（整行宽度 = 128px）
  * @param  page     页坐标，0 ~ 7
  * @param  percent  进度百分比，0 ~ 100
  */
void OLED_DrawProgressBar(uint8_t page, uint8_t percent);

/**
  * @brief  完整显示接口：清屏 → 写两行文字 → 写进度条 → 刷新屏幕
  * @param  angle  当前角度（0 ~ 180）
  *
  * 显示布局：
  *   第 0 页（行 0-7 ）：  "Pos: XXX deg"
  *   第 2 页（行 16-23）：  进度条（[===============   ] 样式）
  */
void OLED_ShowAngle(uint16_t angle);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H */
