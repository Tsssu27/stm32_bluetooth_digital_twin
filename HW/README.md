# STM32F103C8T6 旋转编码器控制舵机工程

## 工程概述

基于 STM32 HAL 库的模块化嵌入式固件工程，实现：
- **HW-040 旋转编码器** (PB0/PB1/PB5) 控制角度
- **SG90 舵机** (PA0, TIM2_CH1 PWM) 执行角度指令
- **0.96" I2C OLED** (PB6/PB7, SSD1306) 实时显示角度和进度条

## 硬件接线

```
STM32F103C8T6       HW-040 编码器
─────────────       ─────────────
PB0            ←→   CLK (A相)    [EXTI0 下降沿]
PB1            ←→   DT  (B相)    [GPIO 输入]
PB5            ←→   SW  (按键)   [EXTI9_5 下降沿]
3.3V           ←→   VCC
GND            ←→   GND

STM32F103C8T6       SG90 舵机
─────────────       ─────────
PA0            ←→   信号线 (黄/橙)
5V             ←→   VCC (红)
GND            ←→   GND (棕/黑)

STM32F103C8T6       0.96" OLED (SSD1306)
─────────────       ──────────────────────
PB6 (I2C1_SCL) ←→   SCL
PB7 (I2C1_SDA) ←→   SDA
3.3V           ←→   VCC
GND            ←→   GND
```

## 目录结构

```
HW/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f1xx_hal_conf.h   # HAL 模块选择配置
│   │   ├── stm32f1xx_it.h
│   │   ├── encoder.h              # 编码器驱动接口
│   │   ├── servo.h                # 舵机驱动接口
│   │   └── oled.h                 # OLED 驱动接口
│   ├── Src/
│   │   ├── main.c                 # 主程序
│   │   ├── system_stm32f1xx.c    # 系统时钟初始化 (72MHz)
│   │   ├── stm32f1xx_it.c        # 中断服务路由
│   │   ├── encoder.c              # 编码器 EXTI 逻辑
│   │   ├── servo.c                # TIM2 PWM 舵机控制
│   │   └── oled.c                 # SSD1306 I2C 显示驱动
│   └── Startup/
│       └── startup_stm32f103c8tx.s  # 汇编启动文件
├── Drivers/
│   ├── CMSIS/                     # ← 需要从 STM32CubeF1 获取
│   └── STM32F1xx_HAL_Driver/      # ← 需要从 STM32CubeF1 获取
├── Makefile
├── STM32F103C8TX_FLASH.ld         # 链接脚本
└── README.md
```

## 软件架构

```
┌─────────────────────────────────────────────┐
│                   main.c                     │
│  while(1): GetAngle → SetAngle → ShowAngle  │
└──────┬──────────────┬───────────────┬────────┘
       │              │               │
  ┌────▼─────┐  ┌─────▼─────┐  ┌─────▼─────┐
  │encoder.c │  │ servo.c   │  │  oled.c   │
  │          │  │           │  │           │
  │ EXTI0    │  │ TIM2 CH1  │  │ I2C1      │
  │ EXTI9_5  │  │ 50Hz PWM  │  │ 400KHz    │
  │ PB0/1/5  │  │ PA0       │  │ PB6/PB7   │
  └──────────┘  └───────────┘  └───────────┘
       ▼              ▼               ▼
    角度值        CCR寄存器        SSD1306帧缓冲
```

## 关键参数

| 参数 | 值 | 说明 |
|------|-----|------|
| SYSCLK | 72 MHz | HSE(8MHz) × PLL9 |
| TIM2 PSC | 71 | 计数频率 = 1 MHz |
| TIM2 ARR | 19999 | PWM 周期 = 20ms = 50Hz |
| CCR@0° | 500 | 脉宽 = 0.5ms |
| CCR@90° | 1500 | 脉宽 = 1.5ms |
| CCR@180° | 2500 | 脉宽 = 2.5ms |
| I2C 速率 | 400 KHz | Fast Mode |
| 编码器步进 | 5° | 每格旋转增量 |
| 轮询周期 | 20ms | 主循环频率 |

## 构建步骤

### 1. 获取 HAL 库

从 ST 官网下载 STM32CubeF1 包，或运行脚本：

```bash
# Linux/macOS
bash get_hal_library.sh

# Windows (PowerShell)
.\get_hal_library.ps1
```

**手动获取步骤：**
1. 访问 https://github.com/STMicroelectronics/STM32CubeF1
2. 下载 ZIP 或 `git clone`
3. 将以下目录复制到本工程：
   - `Drivers/CMSIS/` → `HW/Drivers/CMSIS/`
   - `Drivers/STM32F1xx_HAL_Driver/` → `HW/Drivers/STM32F1xx_HAL_Driver/`

### 2. 安装工具链

```bash
# Ubuntu/Debian
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi

# macOS (Homebrew)
brew install arm-none-eabi-gcc

# Windows: 下载 ARM GNU Toolchain
# https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
```

### 3. 编译

```bash
cd HW
make          # Debug 模式编译
make DEBUG=0  # Release 模式
make size     # 查看 Flash/RAM 使用量
make clean    # 清除构建产物
```

### 4. 烧录

```bash
# 方式一：OpenOCD + ST-Link
make flash

# 方式二：STM32CubeProgrammer
# 打开 build/HW.hex 或 build/HW.bin 烧录

# 方式三：pyOCD
pyocd flash --target stm32f103c8 build/HW.hex
```

## 操作说明

| 操作 | 效果 |
|------|------|
| 旋转编码器（顺时针） | 角度 +5°，舵机转动，OLED 更新 |
| 旋转编码器（逆时针） | 角度 -5°，舵机回转，OLED 更新 |
| 按下编码器按键 | 角度复位至 90°，舵机回中 |
| 角度到达 0° 或 180° | 自动限幅，不溢出 |

## OLED 显示格式

```
┌────────────────┐
│ Pos:  90 deg   │  ← 第 0 页（行 0-7）：角度数值
│                │  ← 第 1 页（空）
│ [════════    ] │  ← 第 2 页（行 16-23）：进度条
│                │
└────────────────┘
```

## 常见问题

**Q: 编译报错 `stm32f103xb.h not found`**
A: 未放置 CMSIS 器件头文件，参考"构建步骤 1"下载 HAL 库。

**Q: 舵机不动或抖动**
A: 检查 PA0 是否接到舵机信号线；确认舵机电源使用独立 5V 供电，不要从 3.3V 引脚取电。

**Q: OLED 无显示**
A: 用逻辑分析仪确认 PB6/PB7 有 I2C 波形；检查 OLED I2C 地址是否为 0x3C（部分模块为 0x3D，修改 `OLED_I2C_ADDR`）。

**Q: 编码器转一格跳多步**
A: `ENCODER_STEP` 宏可调整（当前为 5°），根据实际机械分辨率修改。
