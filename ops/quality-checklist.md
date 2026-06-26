# 出厂质量检验 Checklist

**Pclika MCP Platform — QC 标准检验表 v0.1**

每块板出厂前必须完成全部检验项。操作员在每项后签名/打勾，填写序列号。

---

## 基本信息

| 项目 | 内容 |
|------|------|
| 板卡序列号 | ___________ |
| 生产批次 | ___________ |
| PCB 版本 | ESP32-S3 底板 v0.1 |
| 固件版本 | FW v0.1.0 |
| 检验日期 | ___________ |
| 检验员 | ___________ |

---

## 第一关：目视检查（外观）

| 序号 | 检验项 | 判定标准 | 结果 |
|------|--------|---------|------|
| V01 | PCB 表面清洁 | 无松香残留、无水渍、无异物 | □ PASS □ FAIL |
| V02 | 阻焊层完整 | 无划伤、无气泡、无脱落 | □ PASS □ FAIL |
| V03 | 丝印清晰 | 元件标号、方向标记清晰可读 | □ PASS □ FAIL |
| V04 | 排针/连接器垂直 | 无歪斜、无冷焊 | □ PASS □ FAIL |
| V05 | SMD 元件位置 | 无错位、无立碑、无漏贴 | □ PASS □ FAIL |
| V06 | USB-C 连接器 | 焊接完整，外壳接地点有锡 | □ PASS □ FAIL |
| V07 | ESP32-S3 模组 | 无翘脚，引脚连续可见焊锡 | □ PASS □ FAIL |
| V08 | 按键手感 | BOOT/RST/USER 按键弹性正常 | □ PASS □ FAIL |

---

## 第二关：电气检查（断电状态）

| 序号 | 检验项 | 工具 | 判定标准 | 结果 |
|------|--------|------|---------|------|
| E01 | 3.3V 与 GND 无短路 | 万用表蜂鸣档 | 无蜂鸣 | □ PASS □ FAIL |
| E02 | 5V(VBUS) 与 GND 无短路 | 万用表蜂鸣档 | 无蜂鸣 | □ PASS □ FAIL |
| E03 | USB-C CC1 对 GND：5.1kΩ | 万用表 Ω 档 | 4.8–5.4kΩ | □ PASS □ FAIL |
| E04 | USB-C CC2 对 GND：5.1kΩ | 万用表 Ω 档 | 4.8–5.4kΩ | □ PASS □ FAIL |
| E05 | I2C SDA 上拉（对 3.3V）| 万用表 Ω 档 | 4.3–5.1kΩ | □ PASS □ FAIL |
| E06 | I2C SCL 上拉（对 3.3V）| 万用表 Ω 档 | 4.3–5.1kΩ | □ PASS □ FAIL |

---

## 第三关：上电功能测试

**工具：** USB-C 数据线、测试 PC、pclika-bridge

| 序号 | 检验项 | 操作 | 判定标准 | 结果 |
|------|--------|------|---------|------|
| F01 | 上电无异常 | 接 USB-C，限流 500mA | 电源 LED 亮，无异味，无发热 | □ PASS □ FAIL |
| F02 | USB 枚举 | 观察 PC 设备管理器 | CH340C 识别为 COM 口 | □ PASS □ FAIL |
| F03 | 串口通信 | 115200 baud，按 RST | ESP-IDF 启动 log 可见 | □ PASS □ FAIL |
| F04 | 固件烧录 | 执行 flash_factory.sh | 烧录成功，无 Error | □ PASS □ FAIL |
| F05 | MCP Bridge 连通 | pclika-bridge 启动 | "MCP bridge ready" log | □ PASS □ FAIL |
| F06 | device_info() | MCP 工具调用 | seal = PCK-MMXXVI-C4A32096 | □ PASS □ FAIL |
| F07 | heap_free 检查 | device_info() 返回值 | heap_free > 100000 | □ PASS □ FAIL |
| F08 | Wi-Fi 扫描 | wifi_scan(max_results=3) | 返回 ≥ 1 个网络 | □ PASS □ FAIL |
| F09 | GPIO 输出 | gpio_write(pin=5, level=1) | LED 或万用表确认 | □ PASS □ FAIL |
| F10 | GPIO 输入 | gpio_read(pin=5) | 返回 level 正确 | □ PASS □ FAIL |
| F11 | RGB LED | led_control(r=0,g=200,b=160) | LED 亮青色 | □ PASS □ FAIL |
| F12 | 按键读取 | button_read() | 返回 pressed=false（未按）| □ PASS □ FAIL |

---

## 第四关：I2C 总线测试（如挂传感器）

| 序号 | 检验项 | 操作 | 结果 |
|------|--------|------|------|
| I01 | I2C 总线空载 | I2C scanner，无报错 | □ PASS □ SKIP |
| I02 | SSD1306 OLED（0x3C）| display_text("TEST") | □ PASS □ SKIP |
| I03 | DHT22 温湿度读取 | sensor_read("temp_humidity") | □ PASS □ SKIP |

---

## 第五关：包装前最终确认

| 序号 | 检验项 | 结果 |
|------|--------|------|
| P01 | 合格贴纸已贴（含序列号、FW版本、日期）| □ ✓ |
| P02 | 固件版本已烧录（hello-mcp v0.1.0）| □ ✓ |
| P03 | 防静电袋已密封 | □ ✓ |
| P04 | 配件清单已核对（DHT22/OLED/杜邦线/USB线/卡片）| □ ✓ |
| P05 | 包装盒完好，无损伤 | □ ✓ |

---

## 总评判定

| 全部 PASS | 结论 | 签字 |
|-----------|------|------|
| 是 | **PASS — 可出货** | ___________ |
| 否（有 FAIL 项）| **HOLD — 返工后重测** | ___________ |

**不合格处理：** 标注 FAIL 项，记入 `ops/qa-records/` 不良品记录，转返工工位。

---

## 版本记录

| 版本 | 日期 | 变更内容 |
|------|------|---------|
| v0.1 | 2026-06 | 初始版本 |
