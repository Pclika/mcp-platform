# Pclika Brand Handbook

**版本：** v0.1 | **日期：** 2026-06  
**用途：** 对外合作、设计交付、营销物料制作的标准参考

---

## 1. 品牌核心

### 1.1 品牌定义

**Pclika** 是第一个 AI-native 嵌入式平台 —— 让 Claude、Codex、Cursor 等 AI 编程工具能够直接读传感器、驱动显示屏、控制舵机，和真实世界交互。

一句话：**Import the repo. Flash the board. Claude now talks to real hardware.**

### 1.2 品牌个性

| 维度 | Pclika 的立场 |
|------|-------------|
| 语气 | 直接、精确、不废话 — 像工程师，不像营销人 |
| 态度 | 严肃的可能性 — 我们在做真正有价值的事 |
| 节奏 | 快速迭代，公开建设（Build in public）|
| 美学 | 极简黑色，等宽字体，工业感 + 精密感 |

### 1.3 品牌承诺

- 对开发者：永远 open-source，永远文档优先
- 对客户：出厂经过测试，不售未验证产品
- 对生态：与 AI 工具保持兼容，不做封闭生态

---

## 2. 视觉体系

### 2.1 Logo

**主 Logo：** 文字 Logo「Pclika」，使用 lores-12 字体  
**标志性处理：** 大写 P，其余小写 — 不可全大写，不可全小写

```
✅ Pclika
❌ PCLIKA
❌ pclika
❌ P-clika
```

**Logo 最小尺寸：** 数字端 80px 宽；印刷端 20mm 宽  
**Logo 周围留白：** 不小于字高的 50%

### 2.2 色彩系统

**Primary — 黑色（Black）**

```
HEX: #0A0A0A（近纯黑，避免纯 #000000 的太硬）
RGB: 10, 10, 10
用途: 背景、主色块
```

**Primary Text — 白色（White）**

```
HEX: #F5F5F5
RGB: 245, 245, 245
用途: 主要正文文字
```

**Accent 1 — Teal（青绿）**

```
HEX: #00C8A0
RGB: 0, 200, 160
CSS Var: --teal
用途: CTA 按钮、链接高亮、成功状态、Phase 1 模块标记
```

**Accent 2 — Copper（铜色）**

```
HEX: #C87941
RGB: 200, 121, 65
CSS Var: --copper
用途: 警告、Phase 2 模块标记、ToB 专属标签
```

**Neutral — 深灰（Dark Gray）**

```
HEX: #1A1A1A — 卡片背景
HEX: #2C2C2C — 次级背景
HEX: #666666 — 辅助文字
HEX: #999999 — 占位文字、disabled
```

**禁用色：**

```
❌ 不使用蓝色（避免与平台工具混淆）
❌ 不使用红色作为品牌色（保留给错误状态）
❌ 不使用渐变（降低工业感）
```

### 2.3 字体体系

**标题字体：** lores-12（Adobe Fonts，等宽 bitmap 风格）

```css
font-family: lores-12, monospace;
font-weight: 400;
用途: 大标题、Logo、特殊强调
```

**正文字体：** forma-djr-mono（Adobe Fonts，现代等宽）

```css
font-family: forma-djr-mono, monospace;
font-weight: 400 / 700;
用途: 正文、代码、说明文字
```

**代码字体：** Cascadia Code / JetBrains Mono（fallback）

```css
font-family: "Cascadia Code", "JetBrains Mono", monospace;
用途: 代码块、命令行示例
```

**字体大小规范（Web）：**

| 用途 | 大小 | 字体 |
|------|------|------|
| 超大标题 | 4–8rem | lores-12 |
| 页面标题 | 2–3rem | lores-12 |
| 小节标题 | 1.25rem | forma-djr-mono 700 |
| 正文 | 1rem（16px）| forma-djr-mono 400 |
| 辅助说明 | 0.875rem | forma-djr-mono 400 |
| 代码 | 0.875rem | Cascadia Code |

**中文字体：** 优先继承系统字体（苹方 / 微软雅黑），不强制指定  
中英混排时，英文 / 数字 / 代码使用上述等宽字体，汉字使用系统字体。

### 2.4 间距与布局

- 基础间距单位：8px
- 内容最大宽度：1200px
- 内容区左右内边距：24px（移动端）/ 48px（桌面）
- 卡片圆角：0px（直角，工业感）
- 按钮圆角：0px 或 2px

### 2.5 图形风格

**电路图 / 示意图：**

- 背景：#0A0A0A
- 线条：#00C8A0（teal）或 #C87941（copper）
- 注释文字：白色，等宽字体
- 不使用照片滤镜，使用工程线条图

**产品摄影（打样完成后）：**

- 背景：纯黑或深灰布景
- 补光：单侧冷白灯，突出电路板质感
- 角度：45° 斜上方俯视 + 正面直拍
- 后期：不使用过度 PS，保留真实 PCB 色调

---

## 3. 文字规范

### 3.1 产品命名规范

```
Pclika MCP Platform        → 平台整体名称
pclika-bridge              → Python 包名（全小写带横线）
Pclika ESP32-S3 底板       → 硬件产品（首字母大写）
KIT-MCP-01 Basic           → 套件（全大写 SKU + 描述词）
MCP Basic Kit              → 套件营销名（无 SKU）
```

### 3.2 语气规范

**写作时遵循：**

- 直接说功能，不夸张
- 代码和命令用 `等宽字体`
- 用数字，不用模糊形容词
- 英文技术术语保留英文（MCP, GPIO, UART, I2C）

**示例对比：**

```
❌ "革命性的 AI 硬件体验，让您的开发效率飞速提升！"
✅ "Import the repo. Flash the board. Claude can now read sensors."

❌ "支持多种通信协议，非常灵活"
✅ "支持 UART / I2C / SPI / RS-485，GPIO 20+ 路可用"
```

### 3.3 产品描述模板

```
[产品名] 是 [目标用户] 的 [类型]，
用于 [核心使用场景]。
[关键技术参数1]。[关键技术参数2]。
[一个具体的 MCP 调用示例]。
```

示例：

> KIT-MCP-01 是嵌入式开发者和 AI 工程师的入门套件，用于快速验证 MCP 硬件工作流。ESP32-S3-N16R8，11 个 MCP 工具，开箱即可运行 `sensor_read(sensor_id="temp_humidity")`。

---

## 4. 平台资产

### 4.1 现有资产

| 资产 | 状态 | 格式 |
|------|------|------|
| Logo（文字）| ✅ 完成 | SVG / AI / JPG |
| 网站（5页）| ✅ 完成 | HTML |
| GitHub 仓库 | ✅ 公开 | mcp-platform |
| 字体许可 | ✅ Adobe Fonts | 订阅制 |
| 域名 | ✅ pclika.com | — |

### 4.2 待制作

| 资产 | 优先级 | 备注 |
|------|--------|------|
| 产品 3D 渲染图 | 高 | 打样前可用 KiCad 3D 视图 |
| Demo 视频（3 分钟）| 高 | 参考 `brand/demo-video-script.md` |
| 产品实物照片 | 高 | 打样完成后 |
| GitHub 社交预览图 | 中 | 1280×640，黑底+电路示意 |
| Twitter/X Banner | 中 | 1500×500 |
| 品牌手册 PDF 版 | 低 | 本文档导出 |

---

## 5. 合规与保护

- **商标：** Pclika™ — 参见 `TRADEMARKS.md`
- **Logo 使用限制：** 未经授权不得修改 Logo 形态或颜色
- **开源声明：** 软件 Apache-2.0 / 硬件 CERN-OHL-S / 文档 CC BY 4.0
- **第三方引用：** 可合理引用 Pclika 名称和 Logo 进行评测、教学，须注明来源

---

## 6. 联系与授权

设计合作 / 媒体资源包请联系：**starinvc@gmail.com**  
主题注明：`[BRAND] 请求`
