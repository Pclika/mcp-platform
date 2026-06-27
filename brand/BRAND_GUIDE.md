# Pclika Brand Guide

版本：v1.0  
日期：2026-06-25

---

## 品牌定义

**Pclika** 是一个面向 AI-native 嵌入式开发的开源硬件与开发平台。  
品牌呈现应始终体现：**精密、开放、可扩展、工程感**。

---

## Logo 系统

Pclika 的 Logo 由两个独立单元组成，不得混用或替换。

### 品牌图标（Icon Mark）

文件：`ico.svg`  
形态：白色圆形边框 + 内部 P 字母路径  
用途：Favicon / App 图标 / 头像 / 角标 / 小尺寸场景

```
最小使用尺寸：16px × 16px（数字）/ 5mm（印刷）
安全区：图标尺寸的 20% 四周留白
```

### 字标（Wordmark）

文件：`logo.svg`  
形态：Pclika 完整字母组合，白色路径  
用途：网站 Header / 文档首页 / 产品包装 / 市场材料

```
最小使用尺寸：120px 宽（数字）/ 30mm 宽（印刷）
安全区：字标高度的 50% 四周留白
```

---

## 色彩体系

### 主色

| 名称 | HEX | RGB | 用途 |
|------|-----|-----|------|
| Pclika Black | `#0A0A0A` | 10, 10, 10 | 主背景 / 深色模式 |
| Pclika White | `#F5F5F5` | 245, 245, 245 | Logo / 主文字 |
| Pclika Teal | `#00C8A0` | 0, 200, 160 | 主强调色 / CTA / 高亮 |

### 辅色

| 名称 | HEX | RGB | 用途 |
|------|-----|-----|------|
| Signal Blue | `#0066FF` | 0, 102, 255 | 链接 / 技术标注 |
| Copper | `#C87941` | 200, 121, 65 | 硬件 / PCB 关联元素 |
| Muted Gray | `#3A3A3A` | 58, 58, 58 | 次级背景 / 边框 |
| Light Gray | `#A0A0A0` | 160, 160, 160 | 辅助文字 / 说明 |

### 语义色

| 名称 | HEX | 用途 |
|------|-----|------|
| Success | `#00C87A` | 连接成功 / 编译通过 |
| Warning | `#F5A623` | 警告 / 注意 |
| Error | `#FF3B30` | 错误 / 失败 |
| Info | `#4A9EFF` | 信息 / 提示 |

---

## 字体体系

### 主字体（英文）

**JetBrains Mono** — 代码、技术标注、接口说明  
**Inter** — 正文、产品描述、UI 文字  
**Syne** — 标题、大字号展示文字（首页 Hero）

### 字重规范

| 场景 | 字体 | 字重 |
|------|------|------|
| 大标题 / Hero | Syne | 700 Bold |
| 页面标题 H1/H2 | Inter | 600 SemiBold |
| 正文 | Inter | 400 Regular |
| 代码 / 终端 | JetBrains Mono | 400 Regular |
| 标注 / 标签 | Inter | 500 Medium |

---

## Logo 使用规范

### 允许

- 白色 Logo 在深色背景上
- 黑色 Logo 在浅色/白色背景上（单色印刷场景）
- 等比缩放
- 与平台族标识组合（如 "Pclika ESP32" "Pclika HDL"）

### 禁止

- 拉伸或变形 Logo
- 修改 Logo 颜色（除指定单色版本）
- 在 Logo 上叠加其他图形
- 使用低于最小尺寸
- 截取 Logo 局部单独使用（字标不得截取单个字母）
- 修改字标字母间距

---

## 子品牌规范

Pclika 旗下子项目使用统一前缀 + 功能后缀命名：

| 项目 | 全称 | 短称 |
|------|------|------|
| 主平台 | Pclika MCP Platform | Pclika MCP |
| HDL 平台 | Pclika HDL Platform | Pclika HDL |
| 产品线 | Pclika MCP Ready Kit | Pclika Kit |

子品牌字体处理：正常字重 + 后缀使用 Pclika Teal `#00C8A0` 高亮

---

## 品牌声调

| 维度 | 方向 |
|------|------|
| 技术深度 | 工程精确，不过度简化 |
| 语气 | 直接、简洁、不废话 |
| 态度 | 开放、无傲慢、平等对待所有开发者层次 |
| 避免 | 过度营销语言、空洞形容词、"革命性"等词 |

**核心主张短语：**

> "MCP-ready from the start."  
> "Hardware that AI tools can read."  
> "The layer between hardware and intelligence."

---

## 资产文件清单

| 文件 | 格式 | 说明 |
|------|------|------|
| `ico.svg` | SVG | 品牌图标（圆形 P，矢量） |
| `logo.svg` | SVG | 字标（矢量，含隐藏签名） |
| `logo.jpg` | JPEG | 字标位图版本 |
| `logo.ai` | AI | Illustrator 源文件 |
| `E894CA8186F33379A16C3FCCAF7B31D0.webp` | WebP | 品牌图片资源 |

待补充：
- `logo-dark.svg`（深色背景版，Teal 版）
- `logo-black.svg`（单色黑版，印刷用）
- `logo-white-bg.svg`（带白底矩形版）
- `social-card.png`（1200×630，Open Graph 用）
- `favicon-32.png` / `favicon-16.png`（PNG 版 favicon）
- `app-icon-512.png`（iOS/Android App 图标）

---

## 隐藏签名

所有官方 Pclika 设计资产均嵌有品牌完整性签名。  
详见内部文件 `brand/ORIGIN_CODEX.md`。
