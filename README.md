# STM32 + Python 无线数字孪生控制台 (Wireless Digital Twin Dashboard)

![Python](https://img.shields.io/badge/Python-3.10+-blue?style=for-the-badge&logo=python&logoColor=white)
![C](https://img.shields.io/badge/C-11-blue?style=for-the-badge&logo=c&logoColor=white)
![STM32](https://img.shields.io/badge/STM32-Hardware-green?style=for-the-badge&logo=stmicroelectronics&logoColor=white)
![Bluetooth](https://img.shields.io/badge/Bluetooth-SPP-blue?style=for-the-badge&logo=bluetooth&logoColor=white)

> **物理世界与数字界面的毫秒级双向状态同步**

本项目由 STM32 硬件端与 Python 桌面级可视化上位机组成，旨在提供一套低延迟、高可靠的硬件级数字孪生解决方案。通过经典无线 SPP 蓝牙协议，实现 PC 虚拟控制台和物理机械装置无缝同步运转。

---

## 📂 项目结构 (Monorepo)

```text
stm32_bluttooth_enccoder_steering/
├── HW/             # STM32 C语言固件工程 (含 HAL 库与 Makefile 编译环境)
│   ├── Core/       # 业务逻辑与驱动核心
│   └── build/      # 固件编译输出产物
└── HW_py/          # Python 上位机工程
    └── main.py     # CustomTkinter 双线程控制台入口
```

---

## 🔌 硬件架构与接线表

本系统基于 STM32F103C8T6 构建。核心硬件模块与开发板的接线规范如下：

| 元件名称 | 引脚/功能 | STM32 连接引脚 | 备注说明 |
| :--- | :--- | :--- | :--- |
| **舵机 (SG90 等)** | VCC | 5V (独立电源) | 必须使用独立 5V 供电，切勿直接取芯片 LDO 输出以免欠压重启 |
| | GND | 共地 (GND) | 必须与 STM32 共地 |
| | 信号线 | PA0 | 定时器 PWM 通道控制 |
| **ZS040 (HC-05) 蓝牙** | VCC | 5V / 3.3V | 参考模块背板丝印 |
| | GND | 共地 (GND) | |
| | RX | PA9 (Usart1 TX) | 串口交叉接线 |
| | TX | PA10 (Usart1 RX) | 串口交叉接线 |

---

## 📡 核心通信协议规范

本项目通过 **SPP (Serial Port Profile)** 极简透明传输协议交互。
- **通信接口**: UART (波特率 `9600`, 8数据位, 无校验, 1停止位)

### 1. 下发控制指令 (上位机 -> 下位机)
上位机发送具备首尾标识符和参数名的指令去驱动物理状态。
- **格式**: `<CMD:ANGLE:xxx>\n` (其中 `xxx` 范围通常为 `0` 到 `180`)
- **示例**: `<CMD:ANGLE:45>\n`

### 2. 上报状态指令 (下位机 -> 上位机)
下位机（或编码器反馈）将真实物理状态上报给显示终端，使用原生换行符进行边界分割。
- **格式**: `ANGLE:xxx\n` 
- **示例**: `ANGLE:120\n`

---

## ✨ 核心亮点

*   **上位机：双线程防阻塞架构**
    基于 Python 的上位机采用了现代化的 `customtkinter` 构建丝滑 UI。其底层利用 **独立 Daemon 线程** 专门负责串口非阻塞轮询 (Read) 并将数据安全地投递至 GUI 线程。UI 更新完全不受串口 IO 影响，保障了界面的 60帧 级丝滑响应。
*   **下位机：闭环状态机防跳变逻辑**
    STM32 固件采用了严格的状态机机制去解析 `<CMD:ANGLE:xxx>` 数据流，彻底杜绝了串口乱码、丢包导致的脏数据误操作。有效摒弃了舵机的异常跳变现象，实现鲁棒控制。

---

## 🚀 快速启动指南

### 1. 下位机 (STM32固件) 刷写
1. 进入 `HW/` 目录。
2. 配置好 GCC ARM 编译工具链，通过 `make` 命令生成 `build/` 环境。
3. 将生成的 `.hex` 或 `.bin` 文件通过 ST-Link 或串口 ISP 下载进板载 Flash。
4. 提供独立 5V 外部电源，并复位运行系统。

### 2. 上位机 (Python 控制台) 部署
确保 PC具备 Python 3.10+ 环境。在命令行下执行以下依赖安装：

```bash
pip install customtkinter pyserial
```

进入 `HW_py/` 目录运行：

```bash
python main.py
```

终端启动后请在界面下拉栏选择对应的蓝牙虚拟 COM 端口进行连接。

---

## 🛠️ 常见踩坑排查 (Troubleshooting)

> [!WARNING]
> **Windows 连接蓝牙后瞬间断开？**
> 这是正常的系统级假象。在 Windows 10/11 系统下与 HC-05 蓝牙配对后，Windows 可能会显示“已配对”，但偶尔短暂切到“已连接”后立马回退到“已配对”。
> **解决方案**：此表现为正常逻辑！无需惊慌。系统只是没开透传。您需要去 `设备管理器` -> `端口 (COM 和 LPT)` 中找到 `蓝牙链接上的标准串行 (COMx)` 端口。请务必使用 **“传出方向”** 的那个 COM 端口配置给 Python 上位机。上位机一旦发起 `serial.Serial(COMx)` 建立连接，蓝牙模块上的状态灯才会常亮（此时才物理建立连接）。
