# Demo 视频脚本 — Pclika MCP Platform

**时长目标：** 3 分钟（180 秒）  
**格式：** 屏幕录制 + 实物拍摄 + 旁白  
**平台：** YouTube / X / B站  
**核心信息：** MCP 让 AI 工具真正能操控真实硬件

---

## 分镜结构

### [00:00–00:15] — Hook（悬念开场）

**画面：** 特写 ESP32-S3 底板 + OLED 显示屏 + DHT22，黑色背景，铜色打光

**旁白：**
> "这是一块普通的 ESP32-S3 开发板。但当它连上 Claude，一切就不一样了。"

**字幕叠加：** `sensor_read(sensor_id="temp_humidity")` — 绿色等宽字

---

### [00:15–00:45] — Problem（问题陈述）

**画面：** 分屏：左侧 Claude 聊天界面，右侧一个传统 ESP-IDF 项目（大量代码）

**旁白：**
> "每次想让 AI 帮你调嵌入式代码，它只能猜。它看不到传感器数据。它不知道 GPIO 是高还是低。它在黑盒里工作。Pclika 解决这个问题。"

**字幕：** "AI 需要真实硬件上下文"

---

### [00:45–01:30] — Solution（演示核心流程）

**步骤 1 — Import（10秒）**

> "第一步：在 Claude Code 里导入这个仓库。"

```bash
git clone https://github.com/Pclika/mcp-platform
# 导入到 Claude Code 项目上下文
```

**画面：** 屏幕录制，Claude Code 界面，添加仓库

---

**步骤 2 — Flash（10秒）**

> "第二步：烧录固件。两个命令。"

```bash
cd examples/hello-mcp
idf.py build flash
```

**画面：** 终端窗口，进度条，最后一行：
`I (512) pclika_bridge: MCP bridge ready`

---

**步骤 3 — Connect（10秒）**

> "第三步：注册 MCP 服务器。"

```bash
claude mcp add pclika-bridge -- \
  pip install ./bridge/mcp-server && \
  pclika-bridge --port /dev/cu.usbserial-0001
```

**画面：** Claude Code 显示「pclika-bridge connected」

---

**步骤 4 — Use（20秒）**

> "现在，直接对 Claude 说话。"

**画面：** Claude Code 对话界面，实时录制

```
用户: 读一下温湿度传感器

Claude: 调用 sensor_read(sensor_id="temp_humidity")
        → 返回: { "value": 24.3, "unit": "°C", "timestamp_ms": 12840 }

当前温度 24.3°C，湿度 61.2%。
建议将 OLED 更新为显示当前读数。

用户: 好，显示出来

Claude: 调用 display_text(text="Temp: 24.3°C\nHumidity: 61.2%", clear=True)
        → ok: true
```

**画面（同步）：** 实物 OLED 屏上出现文字 `Temp: 24.3°C`

**旁白：**
> "Claude 直接读到了真实传感器数据。然后直接驱动了真实显示屏。不是模拟，不是猜测 — 是真实的。"

---

### [01:30–02:00] — More（扩展场景）

**画面：** 快速切片（每个 5 秒，无旁白，只有音效和字幕）

```
片段 1: wifi_scan() → 扫描到 8 个 Wi-Fi，列表显示在终端
片段 2: servo_move(channel=0, angle=90) → 舵机实际转动到 90°（实物拍摄）
片段 3: camera_capture() → 返回 base64 图片，Claude 分析图像内容
片段 4: modbus_read(slave_id=1, function="holding_registers", address=0, count=4) → 
         Modbus 仪表数据出现
```

**字幕：** 每个场景底部 → `wifi_scan` / `servo_move` / `camera_capture` / `modbus_read`

---

### [02:00–02:30] — Platform（平台说明）

**画面：** 仓库结构图（静态图，1 秒/条）

**旁白：**
> "Pclika 是一个开放平台。ESP32-S3 底板、23 个 MCP 工具、6 个示例项目。支持 Claude、Codex、Cursor、OpenCode。全部开源，MIT 协议。"

**文字叠加（逐条出现）：**
- `23 MCP tools`
- `ESP32-S3 + sensor / display / motion / vision / industrial`
- `Works with Claude · Codex · Cursor · OpenCode`
- `Open source — github.com/Pclika/mcp-platform`

---

### [02:30–03:00] — CTA（行动号召）

**画面：** 黑色背景，白色文字，Pclika Logo

**旁白：**
> "现在就可以开始。仓库在 GitHub。套件在 pclika.com。"

**字幕：**

```
github.com/Pclika/mcp-platform
pclika.com
```

**结尾帧（3秒 停留）：**

```
Pclika
Import the repo. Flash the board.
Claude now talks to real hardware.
```

---

## 制作备注

### 屏幕录制设置

- 分辨率：1920×1080（导出 1080p）
- 终端字体：Cascadia Code 16px，黑底白字
- Claude Code 界面：保持默认深色主题
- 光标：放大到 150%，方便观看

### 实物拍摄设置

- 背景：黑色无纺布
- 灯光：1 盏冷白 LED 面板灯，左上 45°
- 相机：手机 2x 镜头，固定支架
- 对焦：手动锁定到 PCB 表面

### 后期剪辑

- 软件：CapCut / Premiere Pro
- 字幕：白色等宽字体叠加，左下角
- 代码高亮：teal 色关键词
- 背景音乐：轻微 lo-fi 电子，不喧宾夺主
- 输出：MP4 H.264，10 Mbps，带字幕轨道

### 多语言版本

- 主版本：英文旁白 + 中英双字幕
- 中文版本：中文旁白，面向 B站 / 微信
