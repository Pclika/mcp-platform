# 社交媒体与社区策略

**Pclika MCP Platform — 品牌传播策略 v0.1**

---

## 目标与定位

**核心受众：**
1. AI 工程师 / Prompt 工程师（想让 Claude 做更多的人）
2. 嵌入式开发者（想用 AI 提速的人）
3. 创客 / 独立开发者（想做有实体交互的 AI 项目的人）
4. 大学嵌入式课程教师 / 学生

**传播核心钩子：**
- 🪝 "Claude 现在可以读你的传感器了"
- 🪝 "一条命令，AI 直接控制硬件"
- 🪝 "嵌入式开发的 MCP 层终于有了"

---

## 平台策略

### Twitter / X（主力平台）

**账号：** @Pclika（待注册）  
**发布频率：** 每周 3–5 条  
**内容比例：**

```
40% — 技术演示（GIF / 视频片段 + 代码）
30% — Build in public（进展更新、失败记录、决策背后）
20% % — 社区互动（回复相关讨论、引用 AI 开发者内容）
10% — 产品公告
```

**推文风格模板：**

```
🧵 [演示类]

Claude 刚刚直接读到了我的 I2C 传感器。

[GIF：Claude Code 界面显示 sensor_read() 返回真实数据]

这不是 mock，不是模拟。
是从 ESP32-S3 通过 pclika-bridge 返回的真实数据。

设置步骤：↓
```

```
[Build in public 类]

今天踩坑：ESP32-S3 的 GPIO 34-39 是输入专用，
不能用 gpio_write()。

我们的 troubleshooting 文档加了这条。
感谢 @xxx 的 bug 报告。

链接 → pclika.com/docs/troubleshooting
```

**Hashtag 策略：**
主推：`#MCP` `#ESP32` `#AIHardware` `#ClaudeCode`  
辅助：`#Embedded` `#MakerProject` `#OpenSource`

---

### GitHub（社区核心）

**目标：** 500 Stars（3 个月内）→ 2,000 Stars（6 个月内）

**行动清单：**

- [ ] 完善 README（GIF 演示 + 一键安装徽章 + 示例截图）
- [ ] 添加 GitHub Topics：`mcp`, `esp32`, `esp-idf`, `iot`, `embedded`, `claude`, `ai-hardware`
- [ ] 每个 Release 写 Changelog（用 AI 辅助生成，但人工审核）
- [ ] 及时回复 Issues（目标：24 小时响应）
- [ ] 建立 `good first issue` 标签，降低贡献门槛
- [ ] 发布 Discussions 板块（FAQ / Show & Tell / Feature Requests）

**README 结构优化：**

```markdown
# Pclika MCP Platform

[徽章行：Stars / License / ESP-IDF Version / Python]

> Import the repo. Flash the board. Claude now talks to real hardware.

[GIF：3 秒演示 sensor_read() 返回真实数据]

## Quick Start
[5 行命令]

## What's inside
[23 个工具简表]

## Supported AI tools
[Claude / Codex / Cursor / OpenCode 图标]

[完整文档链接]
```

---

### Hacker News

**发布时机：** v0.1 正式 Release 时（一次性，最高曝光）

**标题模板：**

```
Show HN: Pclika – MCP bridge for ESP32 so Claude can read your sensors
```

**要点：**
- 发布在工作日上午 8–10 点（美国东部时间）
- 在评论区主动回答技术问题
- 准备好：架构说明、为什么不用 WebSocket、为什么选 NDJSON over UART

---

### Reddit

**目标社区：**
- r/MachineLearning（AI 技术向）
- r/esp32（嵌入式向）
- r/arduino（创客向）
- r/embedded（专业嵌入式）
- r/selfhosted（自托管 AI 向）
- r/ClaudeAI（Claude 用户）

**发布策略：**
- 每个社区只发一次（无跨发）
- 格式：技术分享，不是广告（链接到 GitHub，不到产品页）

---

### YouTube

**频道定位：** 技术教程 + Demo 视频

**内容规划：**

| 期数 | 标题 | 时长 |
|------|------|------|
| EP01 | Pclika 平台介绍 — Claude 直接控制 ESP32 | 3 分钟 |
| EP02 | 入门：5 分钟完成 MCP 硬件工作流 | 5 分钟 |
| EP03 | 用 Claude 调试传感器 CRC 错误（真实案例）| 8 分钟 |
| EP04 | 工业场景：Modbus RTU 设备 MCP 接入 | 10 分钟 |
| EP05 | 视觉应用：让 Claude 分析摄像头画面 | 8 分钟 |

---

### 中文社区

**B站：**
- 发布与 YouTube 同步的中文配音版本
- 标题加「Claude」和「ESP32」热词
- 投稿分区：科技 > 数码

**微信公众号（建议名称：Pclika）：**
- 每篇博客转发为图文
- 技术类文章 + 产品更新

**SegmentFault / CSDN / 掘金：**
- 发布 `blog-post-01.md` 的中文译版
- 标题：《为什么我们要为嵌入式开发建一套 MCP 底层》

---

## 发布节奏（前 90 天）

| 时间 | 事件 | 平台 |
|------|------|------|
| Week 1 | GitHub 仓库公开 + README 优化 | GitHub |
| Week 1 | 第一条 Twitter 预告推文 | X |
| Week 2 | Show HN 发布 | Hacker News |
| Week 2 | EP01 Demo 视频上线 | YouTube / X |
| Week 3 | 博客文章 01 发布 | pclika.com / 各社区 |
| Week 4 | Reddit 技术分享（r/esp32 + r/ClaudeAI）| Reddit |
| Month 2 | v0.1 正式 Release + Changelog | GitHub |
| Month 2 | EP02 入门教程上线 | YouTube |
| Month 2 | 中文社区推广（B站 / 掘金）| 中文平台 |
| Month 3 | Product Hunt 发布（配合 v1.0）| Product Hunt |
| Month 3 | KIT 产品上市公告 | 全平台 |

---

## 衡量指标（90 天目标）

| 指标 | 目标 |
|------|------|
| GitHub Stars | ≥ 500 |
| GitHub Forks | ≥ 50 |
| 网站 UV | ≥ 2,000/月 |
| YouTube 订阅 | ≥ 300 |
| Twitter 粉丝 | ≥ 500 |
| 询盘/月 | ≥ 10 条 ToB 询盘 |
