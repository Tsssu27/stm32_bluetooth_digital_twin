/**
  * @file    oled.c
  * @brief   SSD1306 0.96" OLED I2C 驱动实现
  *
  * 实现说明：
  *   - 使用 HAL I2C 阻塞模式（Polling），适合低实时性刷新场景
  *   - 内置 128×64 bit 显存（framebuffer），先写缓冲再整块发送，减少 I2C 传输次数
  *   - 内嵌 5×8 ASCII 字符字体（Space ~ '~'，共 95 个字符）
  *   - I2C 速率：400 KHz（Fast Mode）
  *
  * SSD1306 I2C 帧格式：
  *   [ADDR][Co=0,D/C#=0][CMD] → 命令
  *   [ADDR][Co=0,D/C#=1][DATA...] → 数据流（页数据）
  */

#include "oled.h"
#include <string.h>
#include <stdio.h>

/* ==================== I2C 句柄 =========================================== */
static I2C_HandleTypeDef hi2c1;

/* ==================== 显存（Framebuffer）================================= */
/* 128列 × 8页，每页 8 行像素，共 1024 字节 */
static uint8_t g_oled_buf[OLED_PAGES][OLED_WIDTH];

/* ==================== 5×8 ASCII 字体（从空格 0x20 开始）================= */
/* 每个字符 5 字节，每字节为列数据（bit0=顶行，bit7=底行） */
static const uint8_t font5x8[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' (0x20) */
    {0x00,0x00,0x5F,0x00,0x00}, /* '!' */
    {0x00,0x07,0x00,0x07,0x00}, /* '"' */
    {0x14,0x7F,0x14,0x7F,0x14}, /* '#' */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* '$' */
    {0x23,0x13,0x08,0x64,0x62}, /* '%' */
    {0x36,0x49,0x55,0x22,0x50}, /* '&' */
    {0x00,0x05,0x03,0x00,0x00}, /* ''' */
    {0x00,0x1C,0x22,0x41,0x00}, /* '(' */
    {0x00,0x41,0x22,0x1C,0x00}, /* ')' */
    {0x14,0x08,0x3E,0x08,0x14}, /* '*' */
    {0x08,0x08,0x3E,0x08,0x08}, /* '+' */
    {0x00,0x50,0x30,0x00,0x00}, /* ',' */
    {0x08,0x08,0x08,0x08,0x08}, /* '-' */
    {0x00,0x60,0x60,0x00,0x00}, /* '.' */
    {0x20,0x10,0x08,0x04,0x02}, /* '/' */
    {0x3E,0x51,0x49,0x45,0x3E}, /* '0' */
    {0x00,0x42,0x7F,0x40,0x00}, /* '1' */
    {0x42,0x61,0x51,0x49,0x46}, /* '2' */
    {0x21,0x41,0x45,0x4B,0x31}, /* '3' */
    {0x18,0x14,0x12,0x7F,0x10}, /* '4' */
    {0x27,0x45,0x45,0x45,0x39}, /* '5' */
    {0x3C,0x4A,0x49,0x49,0x30}, /* '6' */
    {0x01,0x71,0x09,0x05,0x03}, /* '7' */
    {0x36,0x49,0x49,0x49,0x36}, /* '8' */
    {0x06,0x49,0x49,0x29,0x1E}, /* '9' */
    {0x00,0x36,0x36,0x00,0x00}, /* ':' */
    {0x00,0x56,0x36,0x00,0x00}, /* ';' */
    {0x08,0x14,0x22,0x41,0x00}, /* '<' */
    {0x14,0x14,0x14,0x14,0x14}, /* '=' */
    {0x00,0x41,0x22,0x14,0x08}, /* '>' */
    {0x02,0x01,0x51,0x09,0x06}, /* '?' */
    {0x32,0x49,0x79,0x41,0x3E}, /* '@' */
    {0x7E,0x11,0x11,0x11,0x7E}, /* 'A' */
    {0x7F,0x49,0x49,0x49,0x36}, /* 'B' */
    {0x3E,0x41,0x41,0x41,0x22}, /* 'C' */
    {0x7F,0x41,0x41,0x22,0x1C}, /* 'D' */
    {0x7F,0x49,0x49,0x49,0x41}, /* 'E' */
    {0x7F,0x09,0x09,0x09,0x01}, /* 'F' */
    {0x3E,0x41,0x49,0x49,0x7A}, /* 'G' */
    {0x7F,0x08,0x08,0x08,0x7F}, /* 'H' */
    {0x00,0x41,0x7F,0x41,0x00}, /* 'I' */
    {0x20,0x40,0x41,0x3F,0x01}, /* 'J' */
    {0x7F,0x08,0x14,0x22,0x41}, /* 'K' */
    {0x7F,0x40,0x40,0x40,0x40}, /* 'L' */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* 'M' */
    {0x7F,0x04,0x08,0x10,0x7F}, /* 'N' */
    {0x3E,0x41,0x41,0x41,0x3E}, /* 'O' */
    {0x7F,0x09,0x09,0x09,0x06}, /* 'P' */
    {0x3E,0x41,0x51,0x21,0x5E}, /* 'Q' */
    {0x7F,0x09,0x19,0x29,0x46}, /* 'R' */
    {0x46,0x49,0x49,0x49,0x31}, /* 'S' */
    {0x01,0x01,0x7F,0x01,0x01}, /* 'T' */
    {0x3F,0x40,0x40,0x40,0x3F}, /* 'U' */
    {0x1F,0x20,0x40,0x20,0x1F}, /* 'V' */
    {0x3F,0x40,0x38,0x40,0x3F}, /* 'W' */
    {0x63,0x14,0x08,0x14,0x63}, /* 'X' */
    {0x07,0x08,0x70,0x08,0x07}, /* 'Y' */
    {0x61,0x51,0x49,0x45,0x43}, /* 'Z' */
    {0x00,0x7F,0x41,0x41,0x00}, /* '[' */
    {0x02,0x04,0x08,0x10,0x20}, /* '\' */
    {0x00,0x41,0x41,0x7F,0x00}, /* ']' */
    {0x04,0x02,0x01,0x02,0x04}, /* '^' */
    {0x40,0x40,0x40,0x40,0x40}, /* '_' */
    {0x00,0x01,0x02,0x04,0x00}, /* '`' */
    {0x20,0x54,0x54,0x54,0x78}, /* 'a' */
    {0x7F,0x48,0x44,0x44,0x38}, /* 'b' */
    {0x38,0x44,0x44,0x44,0x20}, /* 'c' */
    {0x38,0x44,0x44,0x48,0x7F}, /* 'd' */
    {0x38,0x54,0x54,0x54,0x18}, /* 'e' */
    {0x08,0x7E,0x09,0x01,0x02}, /* 'f' */
    {0x0C,0x52,0x52,0x52,0x3E}, /* 'g' */
    {0x7F,0x08,0x04,0x04,0x78}, /* 'h' */
    {0x00,0x44,0x7D,0x40,0x00}, /* 'i' */
    {0x20,0x40,0x44,0x3D,0x00}, /* 'j' */
    {0x7F,0x10,0x28,0x44,0x00}, /* 'k' */
    {0x00,0x41,0x7F,0x40,0x00}, /* 'l' */
    {0x7C,0x04,0x18,0x04,0x78}, /* 'm' */
    {0x7C,0x08,0x04,0x04,0x78}, /* 'n' */
    {0x38,0x44,0x44,0x44,0x38}, /* 'o' */
    {0x7C,0x14,0x14,0x14,0x08}, /* 'p' */
    {0x08,0x14,0x14,0x18,0x7C}, /* 'q' */
    {0x7C,0x08,0x04,0x04,0x08}, /* 'r' */
    {0x48,0x54,0x54,0x54,0x20}, /* 's' */
    {0x04,0x3F,0x44,0x40,0x20}, /* 't' */
    {0x3C,0x40,0x40,0x20,0x7C}, /* 'u' */
    {0x1C,0x20,0x40,0x20,0x1C}, /* 'v' */
    {0x3C,0x40,0x30,0x40,0x3C}, /* 'w' */
    {0x44,0x28,0x10,0x28,0x44}, /* 'x' */
    {0x0C,0x50,0x50,0x50,0x3C}, /* 'y' */
    {0x44,0x64,0x54,0x4C,0x44}, /* 'z' */
    {0x00,0x08,0x36,0x41,0x00}, /* '{' */
    {0x00,0x00,0x7F,0x00,0x00}, /* '|' */
    {0x00,0x41,0x36,0x08,0x00}, /* '}' */
    {0x10,0x08,0x08,0x10,0x08}, /* '~' */
};

/* ==================== 私有函数声明 ======================================= */
static void oled_send_cmd(uint8_t cmd);
static void oled_send_data(uint8_t *data, uint16_t len);
/* oled_set_cursor 仅页寻址模式使用，水平模式下备用，用 unused 属性消除警告 */
static void oled_set_cursor(uint8_t page, uint8_t col) __attribute__((unused));

/* ==================== I2C 初始化 ========================================= */

static void I2C1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* ── 使能时钟 ── */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* ── PB6 (SCL) + PB7 (SDA): 复用开漏输出 ── */
    GPIO_InitStruct.Pin   = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;           /* 开漏 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;      /* 400 KHz 需要高速 */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ── I2C1 参数 ── */
    hi2c1.Instance             = I2C1;
    hi2c1.Init.ClockSpeed      = 400000U;              /* 400 KHz Fast Mode */
    hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1     = 0U;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

/* ==================== SSD1306 初始化命令序列 ============================= */

static const uint8_t s_ssd1306_init_cmds[] = {
    0xAE,        /* Display OFF                      */
    0xD5, 0x80,  /* Clock div ratio / osc freq       */
    0xA8, 0x3F,  /* Multiplex ratio: 64              */
    0xD3, 0x00,  /* Display offset: 0                */
    0x40,        /* Start line: 0                    */
    0x8D, 0x14,  /* Charge pump ON                   */
    0x20, 0x00,  /* Horizontal addressing mode       */
    0xA1,        /* Segment re-map (mirror X)        */
    0xC8,        /* COM scan: remapped               */
    0xDA, 0x12,  /* COM pins hw config               */
    0x81, 0xCF,  /* Contrast: 207                    */
    0xD9, 0xF1,  /* Pre-charge period                */
    0xDB, 0x40,  /* VCOMH deselect level             */
    0xA4,        /* Entire display ON (normal)       */
    0xA6,        /* Normal display (not inverted)    */
    0x2E,        /* Deactivate scroll                */
    0xAF,        /* Display ON                       */
};

/* ==================== 公共函数实现 ======================================= */

void OLED_Init(void)
{
    I2C1_Init();

    HAL_Delay(100U);  /* 等待 OLED 上电稳定 */

    /* 发送初始化命令序列 */
    for (uint16_t i = 0; i < sizeof(s_ssd1306_init_cmds); i++)
    {
        oled_send_cmd(s_ssd1306_init_cmds[i]);
    }

    OLED_Clear();
    OLED_Update();
}

void OLED_Clear(void)
{
    memset(g_oled_buf, 0, sizeof(g_oled_buf));
}

void OLED_Update(void)
{
    /* 设置列地址范围 0~127 */
    oled_send_cmd(0x21);  /* Set Column Address */
    oled_send_cmd(0x00);  /* Column start = 0   */
    oled_send_cmd(0x7F);  /* Column end = 127   */

    /* 设置页地址范围 0~7 */
    oled_send_cmd(0x22);  /* Set Page Address   */
    oled_send_cmd(0x00);  /* Page start = 0     */
    oled_send_cmd(0x07);  /* Page end = 7       */

    /* 一次性发送全部显存（水平寻址模式自动递增） */
    for (uint8_t page = 0; page < OLED_PAGES; page++)
    {
        oled_send_data(g_oled_buf[page], OLED_WIDTH);
    }
}

void OLED_DrawChar(uint8_t x, uint8_t page, char ch, OLED_Color_t color)
{
    if ((uint8_t)ch < 0x20U || (uint8_t)ch > 0x7EU) ch = '?';
    if (x + 6U > OLED_WIDTH || page >= OLED_PAGES) return;

    const uint8_t *glyph = font5x8[(uint8_t)ch - 0x20U];

    for (uint8_t col = 0; col < 5U; col++)
    {
        uint8_t pixels = glyph[col];
        g_oled_buf[page][x + col] = (color == OLED_COLOR_WHITE) ? pixels : (uint8_t)~pixels;
    }
    /* 1 像素列间距 */
    g_oled_buf[page][x + 5U] = (color == OLED_COLOR_WHITE) ? 0x00U : 0xFFU;
}

void OLED_DrawString(uint8_t x, uint8_t page, const char *str, OLED_Color_t color)
{
    while (*str && x < OLED_WIDTH)
    {
        OLED_DrawChar(x, page, *str++, color);
        x += 6U;  /* 5px 字符 + 1px 间距 */
    }
}

void OLED_DrawProgressBar(uint8_t page, uint8_t percent)
{
    if (percent > 100U) percent = 100U;
    if (page >= OLED_PAGES) return;

    /* 外框（两端边界线） */
    g_oled_buf[page][0]           = 0x7EU;  /* 左边框：bit1-bit6 */
    g_oled_buf[page][OLED_WIDTH-1] = 0x7EU; /* 右边框 */

    /* 内部填充区域：第 1~126 列，共 126 像素 */
    uint8_t fill_cols = (uint8_t)((uint16_t)percent * 126U / 100U);

    for (uint8_t col = 1U; col < (uint8_t)(OLED_WIDTH - 1U); col++)
    {
        if (col <= fill_cols)
            g_oled_buf[page][col] = 0x3CU;  /* 填充：bit2-bit5 实心 */
        else
            g_oled_buf[page][col] = 0x24U;  /* 空白：只有边框线 bit2,bit5 */
    }
}

void OLED_ShowAngle(uint16_t angle)
{
    char line1[24];

    OLED_Clear();

    /* ── 第 0 页：显示 "Pos: XXX deg" ── */
    snprintf(line1, sizeof(line1), "Pos: %3u deg", (unsigned)angle);
    OLED_DrawString(0U, 0U, line1, OLED_COLOR_WHITE);

    /* ── 第 2 页：水平进度条（0°=0%, 180°=100%） ── */
    uint8_t percent = (uint8_t)((uint16_t)angle * 100U / 180U);
    OLED_DrawProgressBar(2U, percent);

    OLED_Update();
}

/* ==================== 私有函数实现 ======================================= */

/**
  * @brief  通过 I2C 发送单条 SSD1306 控制命令
  */
static void oled_send_cmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00U, cmd};  /* 0x00 = Co=0, D/C#=0 (命令) */
    HAL_I2C_Master_Transmit(&hi2c1,
                             (uint16_t)(OLED_I2C_ADDR << 1),
                             buf, 2U,
                             OLED_I2C_TIMEOUT);
}

/**
  * @brief  通过 I2C 发送数据流（显存数据）
  * @note   SSD1306 要求数据字节前有 0x40 控制字节
  */
static void oled_send_data(uint8_t *data, uint16_t len)
{
    /* 构建带控制字节的数据包：[0x40][data0][data1]... */
    uint8_t tx_buf[OLED_WIDTH + 1U];
    tx_buf[0] = 0x40U;  /* Co=0, D/C#=1 (数据) */
    memcpy(&tx_buf[1], data, len);

    HAL_I2C_Master_Transmit(&hi2c1,
                             (uint16_t)(OLED_I2C_ADDR << 1),
                             tx_buf, (uint16_t)(len + 1U),
                             OLED_I2C_TIMEOUT);
}

/**
  * @brief  设置 SSD1306 的页/列写入起始光标（页寻址模式）
  */
static void oled_set_cursor(uint8_t page, uint8_t col)
{
    oled_send_cmd(0xB0U | (page & 0x07U));           /* 页地址 */
    oled_send_cmd(0x00U | (col & 0x0FU));             /* 列低4位 */
    oled_send_cmd(0x10U | ((col >> 4) & 0x0FU));      /* 列高4位 */
}

