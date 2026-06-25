# PCB 布线指南 — Pclika ESP32-S3 底板 v0.1

**工程师操作手册 · 预计工时：4–8 小时**

---

## 第一步：用 KiCad 8 打开项目

```
文件路径：hardware/kicad/pclika-esp32s3-v01.kicad_pro
双击 .kicad_pro → KiCad Project Manager 打开
点击 PCB Editor 按钮 → 打开 PCBnew
```

---

## 第二步：从原理图更新 PCB（关键！）

> 这一步将 Schematic 的 netlist（飞线）导入 PCB，使所有 pad 有正确的 net 分配。

```
PCBnew → Tools → Update PCB from Schematic (Ctrl+U)
→ 弹出对话框：保持默认选项
→ 点击 "Update PCB"
→ 点击 "Close"
```

完成后：
- 所有元件出现在板外（堆叠在一起）
- 绿色飞线（ratsnest）连接所有需要连接的 pad
- 状态栏显示 "X unconnected items"

---

## 第三步：元件布局（Place Components）

按照 `component_placement_top.csv` 中的 X/Y 坐标放置元件。
也可以用下面的参考布局：

```
┌────────────────────────────────────────────────────────────┐ 65mm
│  J1(USB-C)  F1  R1 R2  U3(ESD)         C6 C7             │
│  [10,6]     [14,6]      [18,6]                             │ 10mm
│                                                            │
│  U4(CH340C) Q1 Q2  R10 R11  C8 C9                         │
│  [10,18]    [12,24]                                        │ 20mm
│                                                            │
│  U2(LDO)   D1                   U1(ESP32-S3-WROOM-1)      │
│  [10,38]   [6,38]              [32.5, 22.5]                │
│  C1 C2 C3 C4 C5                                            │ 30mm
│                                                            │
│  SW1 SW2 SW3  R12 R13  J2(I2C)  LED1-4  R3 R4             │
│  [56-64,38]           [57,38]   [56-60,6]                  │ 40mm
│                                                            │
│  H1 ○                                              ○ H2   │ 44mm
└────────────────────────────────────────────────────────────┘

ESP32-S3 天线方向：朝向板边（右侧），x>41.5mm 区域无铜
```

### 布局优先级
1. **U1 ESP32-S3** — 先固定，天线朝右，距右边沿 ≥ 3mm
2. **J1 USB-C** — 固定在左边缘，使 USB 口向外
3. **U4 CH340C** — 靠近 J1，UART TX/RX 走线短
4. **U2 AMS1117** — 左下角，去耦电容 C1-C5 紧靠
5. **SW1 RST / SW2 BOOT** — 右下角边缘，易触达
6. **其余被动元件** — 就近原则，平行排列

---

## 第四步：布线规则确认（Design Rules）

文件 `pclika-esp32s3-v01.kicad_dru` 已包含以下规则，确认 PCBnew 已加载：

```
PCBnew → File → Board Setup → Design Rules
```

| 网络类 | 最小线宽 | 最小间距 | 适用网络 |
|--------|---------|---------|---------|
| Default | 0.2mm | 0.15mm | 所有信号线 |
| Power | 0.5mm | 0.2mm | GND / +3V3 / +5V / USB_VBUS |
| USB_Diff | 0.15mm | 0.15mm | USB_D+ / USB_D- |

---

## 第五步：布线顺序（关键路径优先）

### 🔴 优先级 1：USB 差分对（最严格）

```
USB_D+ 和 USB_D- 必须：
- 同层布线（F.Cu）
- 等长（±0.1mm）
- 间距 = 线宽 = 0.15mm
- 不打孔（不换层）
- 远离电源线 ≥ 0.5mm

路径：J1(USB-C) → U3(USBLC6 ESD) → U4(CH340C) UD+/UD-
```

在 PCBnew 中：
```
Route → Interactive Router Settings → Differential Pair Mode: ON
Route → Route Differential Pair (X 键)
选择 USB_D+ pad → 自动配对路由
```

### 🔴 优先级 2：电源网络

```
+5V（VBUS→SS14→Polyfuse→系统）：0.5mm 线宽，或用 F.Cu pour
+3V3：0.5mm 线宽，或用 B.Cu pour / In2.Cu 电源平面
GND：In1.Cu 铜填充（已定义）

电源顺序：
J1.VBUS → F1(Polyfuse) → D1.Anode → D1.Cathode(+5V)
+5V → U2.IN → U2.OUT(+3V3) → C1 C2 C3(去耦)
+3V3 → U1.3V3 / U4.VCC(3.3V端)
```

### 🟡 优先级 3：CH340C 自动下载电路

```
CH340C.DTR → R10(330Ω) → Q1.Base
Q1.Collector → ESP_EN(via R7上拉)
Q1.Emitter → GND

CH340C.RTS → R11(330Ω) → Q2.Base
Q2.Collector → GPIO0_BOOT(via R8上拉)
Q2.Emitter → GND

走线长度：越短越好，≤ 15mm
```

### 🟡 优先级 4：UART 通信

```
U4(CH340C).TXD → U1(ESP32).RXD0 (GPIO44)
U4(CH340C).RXD → U1(ESP32).TXD0 (GPIO43)
线宽：0.2mm，长度 ≤ 30mm
```

### 🟢 优先级 5：I2C、SPI、GPIO

```
I2C：SDA(GPIO8)→J2.pin2, SCL(GPIO9)→J2.pin3  线宽0.2mm
SPI：MOSI(GPIO11), SCK(GPIO12), CS(GPIO13), DC(GPIO14)
WS2812：GPIO48 → R_WS(470Ω) → LED3.DIN
ADC：GPIO1-4 → 测试点（可选）
```

### 🟢 优先级 6：LED 和按键

```
LED1-4：GPIO → R(限流) → LED → GND  0.2mm
SW1(BOOT)：GPIO0 → SW → GND + R8(10k PU to 3V3)
SW2(RST)：EN → SW → GND + R7(10k PU to 3V3)
SW3(USER)：GPIO任意 → SW → GND
```

---

## 第六步：铜填充（Copper Pour）

```
PCBnew → Place → Add Filled Zone (Ctrl+Shift+Z)

Zone 1：In1.Cu → GND（已定义，执行 Fill All 即可）
Zone 2：In2.Cu → +3V3（已定义）
Zone 3：F.Cu  → GND（可选，填充空余区域）
Zone 4：B.Cu  → +3V3（可选）

执行：Edit → Fill All Zones (B 键)
```

ESP32-S3 天线区域（x: 41.5–52mm, y: 10–35mm）铜填充禁布已设置。

---

## 第七步：DRC 检查

```
PCBnew → Inspect → Design Rules Checker (DRC)
→ Run DRC
→ 目标：0 Errors（Warnings 可忽略丝印类）

常见错误处理：
- "Clearance violation" → 调整走线间距
- "Unconnected items" → 有飞线未连接
- "Courtyard overlap" → 调整元件位置
- "Track too narrow" → USB_Diff 检查线宽设置
```

---

## 第八步：3D 视图检查

```
PCBnew → View → 3D Viewer (Alt+3)
检查：元件方向、USB 口方向、天线区域是否干净
```

---

## 第九步：保存并导出

布线完成后运行：

```bash
cd hardware/kicad
./export_gerbers.sh

# macOS：
./export_gerbers.sh /Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli

# Windows (PowerShell)：
./export_gerbers.sh "C:/Program Files/KiCad/8.0/bin/kicad-cli.exe"
```

输出目录：`hardware/kicad/manufacturing/`

---

## 附录：关键元件引脚对应表

### U1 ESP32-S3-WROOM-1 关键引脚

| 引脚名 | GPIO | 连接目标 |
|--------|------|---------|
| 3V3 | — | +3V3 电源 |
| GND | — | GND（多脚） |
| EN | — | ESP_EN（R7上拉，SW2复位） |
| GPIO0 | 0 | GPIO0_BOOT（R8上拉，SW1） |
| GPIO8 | 8 | I2C_SDA（R12上拉） |
| GPIO9 | 9 | I2C_SCL（R13上拉） |
| GPIO11 | 11 | SPI_MOSI |
| GPIO12 | 12 | SPI_SCK |
| GPIO13 | 13 | CS_DISP |
| GPIO14 | 14 | DC_DISP |
| GPIO43 | 43 | UART0_TX → CH340C.RXD |
| GPIO44 | 44 | UART0_RX ← CH340C.TXD |
| GPIO48 | 48 | WS2812_DIN（R_WS串联） |
| GPIO1-4 | 1-4 | ADC1_CH0-3 |

### U2 AMS1117-3.3 引脚

| 引脚 | 网络 |
|------|------|
| 1 ADJ/GND | GND |
| 2 OUTPUT | +3V3 |
| 3 INPUT | +5V |
| TAB | +3V3 |

### U4 CH340C 关键引脚（SOP-16）

| 引脚 | 功能 | 网络 |
|------|------|------|
| UD+ | USB D+ | USB_D+ |
| UD- | USB D- | USB_D- |
| TXD | UART TX out | UART0_RX |
| RXD | UART RX in | UART0_TX |
| DTR# | 下载控制 | → R10 → Q1.B |
| RTS# | Boot控制 | → R11 → Q2.B |
| VCC | 电源 | +5V |
| V3 | 3.3V bypass | +3V3（100nF to GND） |
| GND | 地 | GND |

---

## 完工检查清单

- [ ] DRC 0 Errors
- [ ] USB_D+/D- 等长，间距正确
- [ ] 所有电源线 ≥ 0.5mm
- [ ] GND via 充足（每个IC至少1个热过孔）
- [ ] ESP32 天线区无铜
- [ ] 安装孔 4× M2.5 位置正确
- [ ] USB-C 方向朝板外
- [ ] RST/BOOT 按键在板边易按
- [ ] 丝印无被焊盘遮挡
- [ ] 3D视图外观无明显问题
