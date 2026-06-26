# Pclika KIT 产品线规格书

**版本：** v0.1  
**日期：** 2026-06  
**Seal：** `PCK-MMXXVI-C4A32096`

---

## 产品线总览

| SKU | 名称 | 定位 | 目标用户 | 含税建议零售价 |
|-----|------|------|----------|--------------|
| KIT-MCP-01 | MCP Basic Kit | 入门 / 个人开发者 | 嵌入式初学者、AI 开发者 | ¥399 / $55 |
| KIT-MCP-02 | MCP Pro Kit | 全功能 / 创客 | 产品原型、视觉应用 | ¥1,199 / $165 |
| KIT-MCP-03 | MCP Industrial Kit | 工业级 / ToB | 工厂自动化、IoT 集成 | ¥2,999 / $415（询价为主）|
| KIT-MCP-04 | MCP Edu Lab | 教育 / 5人套装 | 高校、培训机构 | ¥4,499 / $620（5人组） |

---

## KIT-MCP-01 — MCP Basic Kit

### 硬件清单

| 物料 | 规格 | 数量 |
|------|------|------|
| Pclika ESP32-S3 底板 v0.1 | ESP32-S3-WROOM-1-N8R8，USB-C，Type-C 调试 | 1 |
| 温湿度传感器 | DHT22（AM2302），3.3V，±0.5°C | 1 |
| OLED 显示屏 | SSD1306 128×64，I2C，0.96 英寸 | 1 |
| 杜邦线 | 4P 母对母，20cm | 1 包（10 根）|
| USB-C 数据线 | USB-C to USB-A，1m，支持数据传输 | 1 |
| 快速开始卡片 | A6 彩色，正面接线图，背面 QR 直达文档 | 1 |

### 软件内容

- 预装固件：`hello-mcp` + `env-monitor` 示例（含 MCP 工具注册）
- 附件：MCP 客户端配置文件（Claude Code / Cursor / Codex）
- 链接至：`https://pclika.com/docs/quickstart`

### 技术规格

| 项目 | 参数 |
|------|------|
| 主控 | ESP32-S3-WROOM-1-N8R8（240MHz，8MB Flash，8MB PSRAM）|
| 无线 | Wi-Fi 802.11 b/g/n + BLE 5.0 |
| 接口 | USB-C（数据+充电），I2C×2，SPI×1，UART×3，GPIO×20+ |
| 电源 | USB-C 5V 输入，板载 3.3V LDO，可选锂电 |
| 尺寸 | 65mm × 45mm（底板）|
| 固件 | ESP-IDF v5.x，MCP Bridge v0.1.0 |
| MCP 工具 | 11 个基础工具（device_info / sensor_read / display_text 等）|

### 包装

- 外包装：145mm × 95mm × 30mm 灰色哑光纸盒，Pclika Logo 烫黑
- 内衬：黑色 EVA 模切泡棉
- 防静电袋：银色防静电袋（底板）

### 使用场景

- AI 辅助嵌入式学习（Claude Code / Cursor 调试传感器）
- 快速 IoT 原型验证
- MCP 工具开发测试

---

## KIT-MCP-02 — MCP Pro Kit

### 硬件清单

| 物料 | 规格 | 数量 |
|------|------|------|
| Pclika ESP32-S3 底板 v0.1 | ESP32-S3-WROOM-1-N16R8，USB-C | 1 |
| 摄像头模块 | OV2640，200 万像素，JPEG | 1 |
| TFT 显示屏 | ST7789，240×240，SPI，1.3 英寸 | 1 |
| MicroSD 卡槽板 | SPI 接口，支持 FAT32 | 1 |
| 多路舵机驱动 | PCA9685，16 路 PWM，I2C | 1 |
| IMU 传感器 | MPU6050，6 轴，I2C | 1 |
| SG90 舵机 | 180° 范围，3-5V | 2 |
| 杜邦线套装 | 4P/6P 混合，20cm | 1 包 |
| USB-C 数据线 | 1m | 1 |
| 快速开始卡片 | Pro 版，含模块连接示意图 | 1 |

### 软件内容

- 预装固件：`vision-snapshot` + `wifi-scanner` + `hello-mcp`
- 附件：全套 23 工具 MCP 配置 + Prompt 模板
- 额外赠送：`prompts/esp32/` 全套 AI 调试提示词

### 技术规格

| 项目 | 参数 |
|------|------|
| 主控 | ESP32-S3-WROOM-1-N16R8（16MB Flash，8MB PSRAM）|
| 视觉 | OV2640，支持 QVGA → UXGA，JPEG 压缩 |
| MCP 工具 | 全部 23 工具（Base 11 + Sensor 5 + Motion 5 + Vision 4 + Industrial 5）|

---

## KIT-MCP-03 — MCP Industrial Kit

### 硬件清单

| 物料 | 规格 | 数量 |
|------|------|------|
| Pclika ESP32-S3 底板 v0.1 | N16R8，作 MCP 网关 | 1 |
| RS-485 模块 | MAX485，半双工，3.3V 兼容 | 1 |
| 继电器模块 | 8 路，5V，10A/250V AC | 1 |
| 数字输入模块 | 6 路，光耦隔离，5-24V 宽压 | 1 |
| ADC 扩展板 | ADS1115，16 位，4 路差分，I2C | 1 |
| 工业接线端子 | 凤凰端子 3.5mm，12 路 | 1 |
| 导轨安装背板 | DIN 35mm 导轨适配件 | 1 |
| 配套线束 | 工业级屏蔽线，1m | 1 套 |

### 适用场景

- Modbus RTU 设备 MCP 接入
- 工厂继电器控制 AI 化
- 边缘 AI 工业 I/O 网关
- 产线数据采集与 AI 分析

### 销售方式

以 ToB 询价为主，标准报价 ¥2,999；批量（≥10 套）另议。

---

## KIT-MCP-04 — MCP Edu Lab（5 人套装）

### 硬件清单（× 5 份）

| 物料 | 单份规格 | 套装数量 |
|------|---------|---------|
| Pclika ESP32-S3 底板 | N8R8 | 5 |
| 温湿度传感器 DHT22 | 3.3V | 5 |
| OLED 显示屏 SSD1306 | I2C 128×64 | 5 |
| SG90 舵机 | 180° | 5 |
| 基础传感器套件 | 光敏/按键/LED/蜂鸣器 | 5 套 |
| 杜邦线套装 | 40P 混合 | 5 包 |
| USB-C 数据线 | 1m | 5 |

### 附赠内容

- 教师指南 PDF（MCP 实验课程，共 8 课）
- 学生练习题集（每课 5 道练习）
- 班级 GitHub 仓库模板
- 技术支持：1 年在线支持（工作日 48 小时响应）

### 适用场景

- 高校嵌入式 + AI 融合课程
- 职业培训机构
- 创客教育实验室

---

## 套件组合产品线（P2 系列）

| SKU | 套件名 | 核心组件 | 建议价 | 状态 |
|-----|--------|---------|--------|------|
| KIT-A | MCP Starter Kit | ESP32-S3 底板 + DHT22 + OLED | ¥299 | Phase 2 |
| KIT-B | MCP Vision Kit | ESP32-S3-N16R8 + OV2640 + SD 卡 | ¥599 | Phase 2 |
| KIT-C | MCP Motion Kit | ESP32-S3 + PCA9685 + MPU6050 + 2×舵机 | ¥499 | Phase 2 |
| KIT-D | MCP Industrial Gateway | RS485 + CAN + 继电器 + 光耦 DI | ¥1,999 | Phase 3 |
| KIT-E | MCP Edge AI Kit | ESP32-S3 + 麦克风阵列 + OV2640 | ¥799 | Phase 3 |

---

## 单模块零售（P3 系列）

| SKU 前缀 | 类别 | 示例型号 |
|----------|------|---------|
| MOD-SEN-xx | 传感器模块 | MOD-SEN-01 温湿度 / MOD-SEN-02 IMU / MOD-SEN-03 ToF |
| MOD-DSP-xx | 显示模块 | MOD-DSP-01 OLED / MOD-DSP-02 TFT |
| MOD-MOT-xx | 运动模块 | MOD-MOT-01 舵机驱动 / MOD-MOT-02 步进驱动 |
| MOD-CAM-xx | 视觉模块 | MOD-CAM-01 OV2640 |
| MOD-IND-xx | 工业模块 | MOD-IND-01 RS485 / MOD-IND-02 继电器 |

单模块建议零售价：¥59–¥299，按模组复杂度定价。
