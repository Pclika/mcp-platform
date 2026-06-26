# PCBA 生产 SOP

**Pclika MCP Platform — 小批量 PCBA 标准操作程序 v0.1**

适用范围：20–100 件小批量 PCBA 生产（委外嘉立创 SMT 一站式）

---

## 前置条件

- [ ] 原理图已完成并通过审查（H1.1.14）
- [ ] PCB Gerber 文件已导出并通过 DRC
- [ ] BOM v0.1 已确认（`hardware/bom-v0.1.md`）
- [ ] 固件 Binary 已准备好（用于出厂烧录测试）

---

## SOP-S01：PCB 打样前准备

### S01.1 Gerber 文件输出规范

使用 KiCad 8.x 输出：

```
输出文件列表：
├── Gerbers/
│   ├── top_copper.gtl       # 顶层铜箔
│   ├── bottom_copper.gbl    # 底层铜箔
│   ├── inner1_gnd.g2        # 内层 GND
│   ├── inner2_pwr.g3        # 内层 PWR
│   ├── top_silkscreen.gto   # 顶层丝印
│   ├── bottom_silkscreen.gbo
│   ├── top_soldermask.gts   # 顶层阻焊
│   ├── bottom_soldermask.gbs
│   ├── board_outline.gko    # 板型边框
│   ├── drill.drl            # 钻孔文件
│   └── drill_map.gbr        # 钻孔图
├── assembly/
│   ├── top_placement.csv    # 顶面坐标文件（SMT 贴片）
│   └── bom_for_jlcpcb.csv   # 嘉立创 BOM 格式
```

### S01.2 嘉立创 BOM 格式

```csv
Comment,Designator,Footprint,LCSC
ESP32-S3-WROOM-1-N16R8,U1,SMD_18x25.5,C2913197
AMS1117-3.3,U2,SOT-223,C6187
CH340C,U4,SOP-16,C84681
USBLC6-2SC6,U3,SOT-23-6,C2827693
...
```

嘉立创料号（LCSC Part Number）必须填写，否则 SMT 无法自动匹配。

---

## SOP-S02：下单流程（嘉立创 JLCPCB SMT 一站式）

1. **登录** jlcpcb.com → 点击「立即下单」→「SMT贴片」
2. **上传 Gerber**：ZIP 压缩 Gerbers 文件夹上传
3. **PCB 参数设置：**

| 参数 | 值 |
|------|-----|
| 板层 | 4 层 |
| 尺寸 | 65mm × 45mm |
| 数量 | 按需（建议首次 5–10 块）|
| 板厚 | 1.6mm |
| 表面处理 | HASL Lead-Free |
| 阻焊颜色 | 黑色 |

4. **上传 BOM 和坐标文件**（bom_for_jlcpcb.csv + top_placement.csv）
5. **确认料号匹配**：检查每个元件的 LCSC 料号是否正确
6. **贴片面选择**：仅顶面（Bottom 面排针为通孔，手工焊接）
7. **工程费**：首次小批量约 ¥100–¥200
8. **确认费用总计** → 支付 → 记录订单号

---

## SOP-S03：收货检验

**收货时必查项（在签收前完成）：**

- [ ] 包装无破损，防静电措施完好
- [ ] 数量与订单一致
- [ ] 目视检查：无明显焊接缺陷、无漏贴、无错件
- [ ] 用 USB 放大镜或显微镜抽检 20% 板卡的细间距焊点
- [ ] 记录检验结果到 `ops/qa-records/YYYYMMDD-batch-NNN.md`

---

## SOP-S04：通孔元件手工焊接

**适用：排针、排座、按键、连接器（这些嘉立创 SMT 不处理）**

工具：
- 焊台（260°C，斜口烙铁头）
- 助焊剂（免洗型）
- 锡线（0.8mm，含铅或无铅）
- 吸锡器 / 吸锡网

顺序：
1. 先焊低矮元件（电阻、二极管方向）→ 再焊高元件（排针、连接器）
2. USB-C 连接器焊接后用万用表检查 CC1/CC2 是否与 GND 正确连接
3. 所有通孔焊点检查：焊点光亮，无冷焊，孔填充 ≥ 75%

---

## SOP-S05：上电测试流程

**每块板必须逐一执行以下测试：**

### 步骤 1 — 上电前检查

```
□ 目视检查焊接无短路痕迹
□ 万用表蜂鸣档：3.3V 与 GND 之间无短路
□ 万用表蜂鸣档：5V（VBUS）与 GND 之间无短路
```

### 步骤 2 — USB 上电

```
□ 接入 USB-C 电源（限流 USB，最大 500mA）
□ 电源 LED（绿）亮起 → 3.3V 正常
□ 无异常发热（手感 < 50°C）
□ 无焦糊气味
```

### 步骤 3 — USB 枚举测试

```
□ 接 USB 至电脑
□ 设备管理器 / lsusb 能识别到 CH340C（COM 口或 /dev/ttyUSB0）
□ 用串口工具 115200 baud 打开，按 RST 键后看到 ESP-IDF 启动日志
```

### 步骤 4 — 固件烧录

```bash
# 烧录出厂测试固件（hello-mcp）
esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash 0x0 factory_test.bin
```

```
□ 烧录成功（100%，无错误）
□ 重启后串口输出：
  I (512) pclika_bridge: MCP bridge ready
  I (514) pclika_bridge: seal=PCK-MMXXVI-C4A32096
```

### 步骤 5 — MCP 连通验证

```bash
pclika-bridge --port /dev/ttyUSB0
# 调用 device_info()
```

```
□ 返回 board_id、fw_version、seal 正确
□ heap_free > 100000（表示内存正常）
```

### 步骤 6 — Wi-Fi 测试

```
# 调用 wifi_scan(max_results=5)
□ 返回周围 Wi-Fi 列表（至少 1 个网络）
□ rssi 值合理（-100 到 -30 范围内）
```

### 步骤 7 — GPIO 测试

```
# 调用 gpio_write(pin=5, level=1) 再 gpio_read(pin=5)
□ 返回 level=1
# 调用 gpio_write(pin=5, level=0) 再 gpio_read(pin=5)
□ 返回 level=0
```

### 步骤 8 — 记录结果

在测试记录表填写：

| 板卡序号 | 上电 | USB枚举 | 烧录 | MCP | WiFi | GPIO | 结论 |
|---------|------|--------|------|-----|------|------|------|
| NNN-001 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | **PASS** |

---

## SOP-S06：固件批量烧录

生产环境批量烧录使用 `esptool.py` 脚本：

```bash
#!/bin/bash
# 批量烧录脚本 flash_factory.sh
PORT=$1
esptool.py \
  --chip esp32s3 \
  --port $PORT \
  --baud 921600 \
  --before default_reset \
  --after hard_reset \
  write_flash \
  -z \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size detect \
  0x0000 bootloader.bin \
  0x8000 partition-table.bin \
  0x10000 hello_mcp.bin

echo "Flash complete: $PORT"
```

批量时每块板烧录后打印出厂贴纸（序列号二维码）。

---

## SOP-S07：不良品处理

| 缺陷类型 | 处理方式 |
|---------|---------|
| 焊接短路（可修复）| 吸锡返修 → 重测 |
| 元件错位（可修复）| 热风枪拆装 → 重测 |
| PCB 板损坏（不可修）| 报废，记录到废品报告 |
| 功能测试失败（未找到原因）| 隔离，汇报工程师分析 |

废品率目标：< 3%（小批量）/ < 1%（量产）
