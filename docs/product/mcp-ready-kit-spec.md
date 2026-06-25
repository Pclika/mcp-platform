# Pclika MCP Ready Kit Specification

Version: `v0.1`  
Date: `2026-06-25`  
Status: `Foundation release draft`

## 1. Product Definition

Pclika MCP Ready Kit is a hardware and software starter platform for AI-assisted embedded development with MCP-compatible coding tools.

It is intended to help users connect real hardware to tools such as Claude, Codex, Cursor, and OpenCode through the Pclika MCP Platform foundation.

## 2. Product Positioning

- ToC: makers, developers, educators
- ToB: prototype teams, startups, industrial pilot users
- Core value: reduce time to first working embedded workflow

## 3. Design Principles

- no large finished-goods inventory model
- assemble to order
- stable base platform first
- examples, prompts, and docs ship with the hardware concept
- first-generation focus on ESP32-S3

## 4. Product Variants

- `KIT-MCP-01` Basic
- `KIT-MCP-02` Pro
- `KIT-MCP-03` Industrial
- `KIT-MCP-04` Edu Lab

## 5. Base Hardware

- ESP32-S3
- USB-C power and debug
- Wi-Fi and BLE
- onboard LED and button
- sensor and display-ready interfaces

## 6. Software Model

The product is built around:

- device runtime
- MCP bridge
- tool schemas
- build and flash flow
- examples and prompts

## 7. Starter Tool Surface

- `device_info`
- `led_control`
- `button_read`
- `sensor_read`
- `display_text`
- `wifi_scan`
- `gpio_write`
- `gpio_read`
- `serial_log_read`
- `firmware_version`

## 8. Initial Success Criteria

- 10 minutes to first demo
- 30 minutes to first AI-assisted change
- 1 day to first independent prototype direction

## 9. Related Documents

- [Core platform](../architecture/core-platform.md)
- [Base platform](../hardware/base-platform.md)
- [MCP development flow](../software/mcp-development-flow.md)
