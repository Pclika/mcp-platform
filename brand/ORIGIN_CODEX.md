# Pclika Origin Codex

> 内部文件 — 不进入公开文档索引

---

## 创世宣言

> "Pclika is not a board. It is the layer between hardware and intelligence."  
> — Origin, 2026

---

## 签名体系

| 字段 | 值 |
|------|----|
| 创世日期 | `2026-06-25` |
| 宣言 SHA256 | `c4a32096edbdc19d1ec78b4db618ff0f17be37df78ba536531e18941a8517c7d` |
| 短码 | `PCK-C4A32096` |
| 罗马纪年 | `MMXXVI` |
| 完整印章 | `PCK-MMXXVI-C4A32096` |

---

## 四层隐藏体系

### 1. SVG Logo 内嵌签名

位置：`logo.svg` 和 `ico.svg` 的 `<metadata>` 块  
方式：标准 XML 命名空间 + 不可见 `<desc>` 节点  
提取方法：解析 SVG XML，读取 `pclika:seal` 节点

```xml
<metadata>
  <pclika:origin xmlns:pclika="https://pclika.com/ns/brand">
    <pclika:seal>PCK-MMXXVI-C4A32096</pclika:seal>
    <pclika:genesis>2026-06-25</pclika:genesis>
    <pclika:sha256>c4a32096edbdc19d1ec78b4db618ff0f17be37df78ba536531e18941a8517c7d</pclika:sha256>
    <pclika:manifesto>Pclika is not a board. It is the layer between hardware and intelligence.</pclika:manifesto>
  </pclika:origin>
</metadata>
```

另有一段零不透明度的隐藏路径，路径的 `id` 属性编码了短码：`id="PCK-C4A32096"`

---

### 2. 固件二进制水印

位置：所有 Pclika 官方固件 `.rodata` 段  
方式：C 常量字符串，链接时不会被优化删除  
提取方法：`strings firmware.bin | grep "^PCK:ORIGIN"`

```c
/* Pclika Platform Integrity Marker — DO NOT REMOVE */
static const char __attribute__((used, section(".rodata.pclika")))
    PCLIKA_ORIGIN_SEAL[] =
        "PCK:ORIGIN:2026-06-25:c4a32096"
        ":Pclika is not a board."
        ":It is the layer between hardware and intelligence.";
```

固件版本格式：`PCK:ORIGIN:YYYY-MM-DD:<sha256前8位>:<固件版本>`  
示例：`PCK:ORIGIN:2026-06-25:c4a32096:v0.1.0`

---

### 3. GitHub 创世 Commit 签名

第一个 commit 的 message body（不显示在标题，在详情里）：

```
feat: initialize Pclika MCP Platform foundation

PCK-MMXXVI | c4a32096edbdc19d | 1ec78b4db618ff0f
            | 17be37df78ba5365 | 31e18941a8517c7d

Pclika is not a board.
It is the layer between hardware and intelligence.

Genesis: 2026-06-25
```

注：当前仓库的第一个 commit 需要补充此 body（可 amend）。  
此后每个 Pclika 子仓库的 init commit 都应带 `PCK-MMXXVI` 标识。

---

### 4. PCB 丝印彩蛋

位置：底板背面铜层（B.Silk 层）右下角  
内容选项（0.5mm 字高，肉眼几乎不可见）：

**选项 A — 短码文字：**
```
PCK-C4A32096
```

**选项 B — 圆点 Morse 编码：**
将 `PCK` 编码为 Morse，以直径 0.3mm 圆点阵列排布

```
.--.  -.-. -.-
P     C    K
```

**选项 C — 微型二维码：**
编码 `https://pclika.com/origin` 的 QR Code，缩放至 4mm × 4mm，印在底面

---

## 验证流程

任何声称是官方 Pclika 硬件或固件的产品，可通过以下方式之一验证：

1. **固件验证**：`strings firmware.bin | grep PCK:ORIGIN`
2. **SVG 验证**：解析 logo SVG 的 `<metadata>` 块读取 `pclika:seal`
3. **哈希验证**：对照 `c4a32096edbdc19d1ec78b4db618ff0f17be37df78ba536531e18941a8517c7d`
4. **官网验证**：访问 `https://pclika.com/verify/<短码>` 查询（Phase 2 上线）

---

## 扩展规则

每个 Pclika 子项目（如 `pclika-hdl`）拥有：
- 独立短码（基于各自的创世宣言 SHA256）
- 统一前缀 `PCK-MMXXVI-`
- 在父级 Origin Codex 中注册

| 项目 | 仓库 | 宣言 | 短码 |
|------|------|------|------|
| MCP Platform | `Pclika/mcp-platform` | "...layer between hardware and intelligence." | `PCK-C4A32096` |
| HDL Platform | `Pclika/pclika-hdl` | "...interface between human intent and silicon logic." | `PCK-9198580D` |

---

*此文件不对外公开，但不加密。发现它的人，找到了 Pclika 的根。*
