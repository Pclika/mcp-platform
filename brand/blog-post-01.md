# 为什么我们要为嵌入式开发建一套 MCP 底层

**Pclika — 首篇技术博客**  
**作者：** Pclika Team  
**发布日期：** 2026-06  
**标签：** MCP, ESP32, AI hardware, embedded systems, Claude, Codex

---

这篇文章解释我们为什么在做 Pclika，以及我们认为 AI 和嵌入式开发结合的正确方式是什么。

---

## 问题：AI 工具在硬件开发中是盲人

过去一年，AI 编程助手（Claude Code、Cursor、Copilot、Codex）在软件开发领域的生产力提升是真实的。但如果你做嵌入式开发，你很快会发现一堵墙：

**AI 看不到硬件状态。**

你问 Claude："我的 DHT22 为什么返回 NaN？"它会猜：可能是接线、可能是时序、可能是电源。它没法知道 GPIO 的实际电平。它没法读出传感器的实际原始值。它无法访问串口 log。它在黑盒里给你建议。

这不是 AI 的问题 — 这是工具链层面缺少一个接口。

---

## 发现：MCP 是解法的一半

2024 年底，Anthropic 发布了 [Model Context Protocol (MCP)](https://docs.anthropic.com/mcp)。本质上，它允许开发者把工具暴露给 AI 模型 —— 定义一个工具，AI 就可以调用它，就像 function call 一样。

我们当时意识到：**如果把硬件操作封装成 MCP 工具，AI 就能真正"操控"硬件了。**

```
sensor_read(sensor_id="temp_humidity") 
  → returns { value: 24.3, unit: "°C" }
```

这不是模拟，不是假数据 — 这是从真实传感器读出的真实数值，通过 UART 传给 AI，AI 可以基于这个决策、调试、编写代码。

但 MCP 只定义了协议层。硬件侧的 "另一半" 还不存在。

---

## 解法：Pclika MCP Platform

我们建了 Pclika 来填补这一半：

```
AI 工具 (Claude/Codex/Cursor)
    ↕ MCP (JSON-RPC 2.0 over STDIO)
pclika-bridge (Python)
    ↕ NDJSON over USB-Serial
ESP32-S3 固件 (ESP-IDF v5.x + FreeRTOS)
    ↕ I2C / SPI / UART / GPIO
真实传感器 / 显示屏 / 电机 / 继电器
```

三层，每层可以独立存在，也可以一起运转。

**设备层**用 ESP-IDF 写驱动和工具处理函数，注册 23 个 MCP 工具。  
**桥接层**是一个 Python STDIO 服务器，把 JSON-RPC 2.0 翻译成 NDJSON 发送到串口。  
**AI 层**配置好 MCP 服务器后，直接在 Claude Code 或 Cursor 里调用工具。

这样设计有几个好处：

1. **AI 工具无感知** — Claude Code、Cursor、Codex 都使用同一个 MCP 协议，pclika-bridge 对每个工具的接口完全一样
2. **硬件平台可替换** — 只要固件实现相同的 NDJSON 协议，换 STM32 或 RP2040 也能跑
3. **工具 Schema 标准化** — 每个工具有 inputSchema + outputSchema，AI 知道参数和返回值的精确格式

---

## 真实体验是什么样的

安装流程：

```bash
git clone https://github.com/Pclika/mcp-platform
cd examples/hello-mcp && idf.py build flash
pip install ./bridge/mcp-server
claude mcp add pclika-bridge -- pclika-bridge --port /dev/ttyUSB0
```

之后，在 Claude Code 里：

```
我: 帮我写一个循环监测温度，超过 28°C 就让 LED 变红

Claude: 先读一下当前温度
        [调用 sensor_read(sensor_id="temp_humidity")]
        返回: 24.3°C
        
        好，当前 24.3°C。我来写这个逻辑：
        [给出 FreeRTOS 任务代码，带 led_control() MCP 调用]
        
        要我直接烧录测试版吗？
```

AI 实时知道传感器读数。它可以做对比、做验证、做决策。这和以前"猜测"的体验完全不同。

---

## 我们做了什么

目前 mcp-platform 包含：

- **23 个 MCP 工具**：覆盖传感器读取、显示屏控制、伺服电机、视觉（摄像头）、工业 I/O（Modbus RTU、继电器）
- **4 个传感器驱动**：SHT31、BH1750、MPU6050、VL53L0X
- **2 个显示驱动**：SSD1306 OLED（I2C）、ST7789 TFT（SPI）
- **6 个示例项目**：从 hello-mcp 到 industrial-gateway
- **完整工具 Schema**：inputSchema + outputSchema，支持 AI 自动类型推导
- **4 个 AI 提示词模板**：系统上下文、传感器调试、固件 Review、工业场景

---

## 接下来

Phase 2 我们会做硬件。ESP32-S3 定制底板（65mm × 45mm，4 层 PCB）和标准化的扩展接口 —— 这样传感器模块可以即插即用，不用查引脚手册。

然后是 KIT：开箱即用的套件，里面有板卡、传感器、OLED、线，以及一张快速开始卡片，扫码直达文档。

---

## 为什么开源

因为 MCP 生态应该是开放的。封闭的硬件平台限制了 AI 工具能接入的范围，也限制了开发者能做的事情。Pclika 选择 Apache-2.0（软件）+ CERN-OHL-S（硬件）+ CC BY 4.0（文档）。

我们认为 MCP + 开放硬件是未来 AI 辅助开发的基础设施。我们想做对的那一层。

---

**GitHub：** [Pclika/mcp-platform](https://github.com/Pclika/mcp-platform)  
**网站：** [pclika.com](https://pclika.com)  
**邮件：** starinvc@gmail.com
