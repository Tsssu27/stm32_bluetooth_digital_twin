##############################################################################
# Makefile — STM32F103C8T6 模块化工程
# 目标芯片 : STM32F103C8T6  (Cortex-M3, 72MHz, 64KB Flash, 20KB RAM)
# 工具链   : arm-none-eabi-gcc
# 构建模式 : Debug (可切换为 release)
#
# 用法：
#   make          — 编译生成 build/HW.elf / .hex / .bin
#   make clean    — 清除构建产物
#   make flash    — 通过 ST-Link 烧录 (需安装 openocd)
#   make size     — 打印各 section 大小
##############################################################################

######################################
# 目标名称
######################################
TARGET = HW

######################################
# 构建目录
######################################
BUILD_DIR = build

######################################
# 调试 / 发布开关
# DEBUG=1 → -Og -g (默认)
# DEBUG=0 → -O2
######################################
DEBUG = 1

######################################
# 工具链前缀
######################################
PREFIX = arm-none-eabi-
CC      = $(PREFIX)gcc
CXX     = $(PREFIX)g++
AS      = $(PREFIX)gcc -x assembler-with-cpp
CP      = $(PREFIX)objcopy
SZ      = $(PREFIX)size
HEX     = $(CP) -O ihex
BIN     = $(CP) -O binary -S

######################################
# 目标芯片 (Cortex-M3, 无 FPU)
######################################
CPU      = -mcpu=cortex-m3
THUMB    = -mthumb
# M3 无硬件浮点：
FPU      =
FLOAT_ABI=

MCU = $(CPU) $(THUMB) $(FPU) $(FLOAT_ABI)

######################################
# 链接脚本
######################################
LDSCRIPT = STM32F103C8TX_FLASH.ld

######################################
# C 源文件列表
######################################
C_SOURCES = \
  Core/Src/main.c                                    \
  Core/Src/encoder.c                                 \
  Core/Src/servo.c                                   \
  Core/Src/oled.c                                    \
  Core/Src/usart.c                                   \
  Core/Src/stm32f1xx_it.c                            \
  Core/Src/system_stm32f1xx.c                        \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c                     \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c                 \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c              \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c                \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c             \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c               \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c            \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c                 \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c              \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c                \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c                 \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c              \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c                \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c                 \
  Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c

######################################
# 汇编启动文件
######################################
ASM_SOURCES = Core/Startup/startup_stm32f103c8tx.s

######################################
# 头文件搜索路径
######################################
C_INCLUDES = \
  -ICore/Inc                                                             \
  -IDrivers/STM32F1xx_HAL_Driver/Inc                                    \
  -IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy                             \
  -IDrivers/CMSIS/Device/ST/STM32F1xx/Include                           \
  -IDrivers/CMSIS/Include                                               \
  -IDrivers/CMSIS/Core/Include

######################################
# 预处理宏定义
######################################
C_DEFS = \
  -DUSE_HAL_DRIVER          \
  -DSTM32F103xB             \
  -DUSE_FULL_ASSERT

######################################
# 编译标志
######################################
ifeq ($(DEBUG), 1)
  OPT = -Og -g -gdwarf-2
else
  OPT = -O2
endif

# 公共编译参数
COMMON_FLAGS = $(MCU) $(OPT) \
  -Wall -Wextra -Wshadow -Wdouble-promotion \
  -ffunction-sections -fdata-sections \
  -fstack-usage \
  --specs=nano.specs

CFLAGS  = $(COMMON_FLAGS) $(C_DEFS) $(C_INCLUDES) \
  -std=gnu11

ASFLAGS = $(MCU) $(OPT) -x assembler-with-cpp \
  $(C_DEFS) $(C_INCLUDES) \
  -Wall -fdata-sections -ffunction-sections

######################################
# 链接标志
######################################
LIBS    = -lc -lm -lnosys
LIBDIR  =
LDFLAGS = $(MCU) \
  -specs=nano.specs \
  -T$(LDSCRIPT)     \
  $(LIBDIR) $(LIBS) \
  -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
  -Wl,--gc-sections \
  -Wl,--print-memory-usage

######################################
# 对象文件推导规则
######################################
OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

# vpath — 让 make 在各子目录中找源文件
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

######################################
# 默认目标
######################################
.PHONY: all clean flash size

all: $(BUILD_DIR)/$(TARGET).elf \
     $(BUILD_DIR)/$(TARGET).hex \
     $(BUILD_DIR)/$(TARGET).bin

######################################
# 编译 C 文件
######################################
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "[CC]  $<"
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

######################################
# 汇编启动文件
######################################
$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "[AS]  $<"
	@$(AS) -c $(ASFLAGS) $< -o $@

######################################
# 链接
######################################
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "[LD]  $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

######################################
# 生成 HEX / BIN
######################################
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "[HEX] $@"
	@$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "[BIN] $@"
	@$(BIN) $< $@

######################################
# 创建构建目录
######################################
$(BUILD_DIR):
	mkdir -p $@

######################################
# 清除
######################################
clean:
	@echo "[CLN] Removing build directory..."
	@rm -rf $(BUILD_DIR)

######################################
# 打印 section 大小
######################################
size: $(BUILD_DIR)/$(TARGET).elf
	@$(SZ) --format=berkeley $<

######################################
# 烧录（OpenOCD + ST-Link）
######################################
flash: $(BUILD_DIR)/$(TARGET).hex
	@echo "[FLASH] Programming via OpenOCD..."
	openocd \
	  -f interface/stlink.cfg \
	  -f target/stm32f1x.cfg  \
	  -c "program $(BUILD_DIR)/$(TARGET).hex verify reset exit"

######################################
# 依赖文件（增量编译）
######################################
-include $(OBJECTS:.o=.d)

CFLAGS += -MMD -MP
