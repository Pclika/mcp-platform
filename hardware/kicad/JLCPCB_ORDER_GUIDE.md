# JLCPCB 下单指南 — Pclika ESP32-S3 底板 v0.1

**PCB + SMT 贴装一站式服务**

---

## 前提条件

- [ ] KiCad 8 布线完成（见 `ROUTING_GUIDE.md`）
- [ ] DRC 0 Errors 通过
- [ ] 运行 `./export_gerbers.sh` 生成 `manufacturing/` 目录

---

## 第一步：PCB 参数设置

访问：**https://jlcpcb.com** → 点击 **Instant Quote**

上传 `manufacturing/pclika-esp32s3-v01-gerbers.zip`，等待解析完成。

设置以下参数：

| 参数 | 值 | 说明 |
|------|-----|------|
| Base Material | FR-4 | 标准 |
| Layers | **4** | 四层板 |
| Dimensions | **65 × 45 mm** | 自动解析，核对 |
| PCB Qty | **50** | 最小经济批量 |
| PCB Thickness | **1.6 mm** | 标准厚度 |
| PCB Color | **Black** | Pclika 品牌色 |
| Silkscreen | White | 白色丝印 |
| Surface Finish | **ENIG** | 化金，提升焊接质量 |
| Outer Copper Weight | 1 oz | 标准 |
| Inner Copper Weight | 0.5 oz | 内层标准 |
| Via Covering | Tented | 标准 |
| Board Outline Tolerance | ±0.2mm | 标准 |
| Confirm Production file | Yes | 确认生产文件 |
| Remove Order Number | Specify a location | 指定丝印层位置（或选择 No） |

> ⚠ **Surface Finish**：ENIG（化金）比 HASL 贵约 ¥30，但焊接质量更好，推荐用于原型。  
> 如预算紧可改为 **HASL (Lead-free)**，约节省 ¥20-30/50pcs。

---

## 第二步：开启 SMT Assembly

在 PCB 参数下方，找到 **PCB Assembly** 区域：

```
PCB Assembly  ✅ 开启
Assembly Side: Top Side
PCBA Qty: 50（与 PCB 数量一致）
Tooling holes: Added by JLCPCB
Confirm Parts Placement: Yes（强烈建议）
```

点击 **Confirm** 进入 Assembly 页面。

---

## 第三步：上传 BOM 和坐标文件

**上传 BOM：**
```
点击 "Add BOM File"
选择：manufacturing/bom/jlcpcb_bom.csv
```

**上传坐标文件：**
```
点击 "Add CPL File" (Component Placement List)
选择：manufacturing/bom/component_placement_top.csv
```

点击 **Process BOM & CPL**，等待解析（约 30 秒）。

---

## 第四步：元件确认

系统会显示每个元件的匹配状态：

| 状态 | 含义 | 处理 |
|------|------|------|
| ✅ 绿色 | 自动匹配 LCSC 库存 | 无需操作 |
| 🟡 黄色 | 多个候选，需选择 | 按 LCSC 编号选择 |
| 🔴 红色 | 无库存或未匹配 | 手动搜索替代品 |
| ⬜ No Placement | 不焊接（如 THT） | 保持 |

### 关键元件核对清单

| 位号 | LCSC编号 | 核对点 |
|------|---------|--------|
| U1 | C2913197 | ESP32-S3-WROOM-1-**N16R8**（不是 N8） |
| U4 | C84681 | CH340**C**（不是 CH340G/N） |
| U3 | C2827693 | USBLC6-2**SC6**（SOT-23-6封装） |
| C1 | C16780 | 100μF 10V 电解（注意极性方向） |
| LED3 | C114586 | WS2812B-**2020**（不是 5050） |

### 常见替代情况

**如果 ESP32-S3-WROOM-1-N16R8（C2913197）库存不足：**
- 联系安信可（ai-thinker.com）或深圳代理商
- 搜索 LCSC：`ESP32-S3-WROOM-1U-N16R8`（外置天线版，需另焊天线）

**如果 CH340C（C84681）库存不足：**
- 可换用 CH340E（更小封装，需更新原理图）或 CP2102N

---

## 第五步：坐标方向确认

**3D 预览检查（非常重要）：**

```
点击页面右上角 "3D Viewer" 或 "Preview"
```

检查点：
- [ ] U1 ESP32-S3 天线端朝向板边（右侧）
- [ ] J1 USB-C 接口朝向板边（左侧）
- [ ] U4 CH340C 方向正确（Pin 1 标记位置）
- [ ] C1 电解电容极性正确（正极标记）
- [ ] LED1-4 方向正确
- [ ] SW1 SW2 SW3 位置在板边缘

如有方向错误，回到坐标文件修改 Rotation 值（每次 +90°/-90°）。

---

## 第六步：THT 元件（手焊）

以下元件需要手焊或不含在 SMT 服务中：

| 元件 | 说明 | 来源 |
|------|------|------|
| J2 JST SH 4P（卧贴） | 可选 SMT 贴装 | LCSC C160404 |
| 安装孔 H1-H4 | 无需焊接（走位孔） | — |

> 所有元件均为 SMD，理论上都可 SMT，但 JST 连接器有时 JLCPCB 不代贴，需手焊。

---

## 第七步：费用预算（50pcs 参考）

| 项目 | 费用（参考） |
|------|------------|
| PCB 制造（4层，黑，ENIG，50片）| ¥180–220 |
| SMT 贴装费（基础）| ¥80–120 |
| 元件成本（50片总，按BOM）| ¥3,200–3,600 |
| **合计（含运费）** | **¥3,500–4,000** |
| **单板成本** | **¥70–80** |

> 注：ESP32-S3-WROOM-1-N16R8 价格约 ¥40-50/片，是最大单项成本。

---

## 第八步：下单后流程

```
1. 提交订单 → 等待工程文件确认（1工作日）
2. JLCPCB 发邮件确认 → 核对确认图
3. 生产周期：PCB 2-3天 + SMT 3-5天
4. 快递 → 到货验货

到货后：
5. 抽3块 → 运行质检 Checklist (ops/quality-checklist.md)
6. 通过 → 批量上固件 (ops/pcba-sop.md flash_factory.sh)
7. 全检 → 合格贴标 → 包装入库
```

---

## 第九步：常见问题

**Q：板子是否需要 V-Cut / 拼版？**  
A：65×45mm 单板，50片直接下单，无需拼版。如需节省运费可 2×2 拼版。

**Q：Gerber 文件层名对应？**

| JLCPCB 层 | Protel 扩展名 | 说明 |
|-----------|-------------|------|
| Top Copper | .GTL | F.Cu |
| Bottom Copper | .GBL | B.Cu |
| Inner Layer 1 | .G2 / .IN1 | In1.Cu（GND平面）|
| Inner Layer 2 | .G3 / .IN2 | In2.Cu（+3V3平面）|
| Top Soldermask | .GTS | F.Mask |
| Bottom Soldermask | .GBS | B.Mask |
| Top Silkscreen | .GTO | F.Silkscreen |
| Board Outline | .GKO | Edge.Cuts |
| Drill | .DRL | Through-hole钻孔 |

**Q：ENIG 和 HASL 有什么区别？**  
A：ENIG（化镍金）平整度更好，适合 QFP/QFN/BGA 焊接，焊点质量更稳定。推荐原型使用 ENIG，量产可换 HASL 降成本。

---

## 联系备用供应商

| 物料 | 供应商 | 联系方式 |
|------|--------|---------|
| ESP32-S3 模组 | 安信可 ai-thinker.com | sales@ai-thinker.com |
| PCB+PCBA | JLCPCB | support@jlcpcb.com |
| 被动元件 | LCSC lcsc.com | — |
| 包装耗材 | 淘宝/1688 | 搜索"防静电袋 ESP32" |
