# Pclika MCP Platform Architecture

## Purpose

Pclika MCP Platform is the shared technical foundation for:

- MCP-ready hardware kits
- Open-source embedded development
- AI-assisted firmware creation
- Website-based guided product selection
- Long-term platform expansion

## Platform Definition

The platform consists of five layers.

### 1. Hardware Base Layer

This layer defines the standard MCU board and expansion interface.

Initial baseline:

- MCU family: `ESP32-S3`
- Connectivity: `Wi-Fi`, `BLE`
- Development interface: `USB-C`
- Base buses: `GPIO`, `I2C`, `SPI`, `UART`, `PWM`, `ADC`
- User IO: `LED`, `Button`
- Expansion interface: standard module headers for sensor, display, motion, and industrial communication
- Reliability baseline: memory, wireless, interface, and security rules defined at the platform level

### 2. Device Runtime Layer

This layer abstracts hardware into reusable runtime modules.

Initial runtime domains:

- System
- GPIO
- Sensor
- Display
- Motion
- Connectivity
- Logging
- Device configuration

### 3. MCP Tool Layer

This layer exposes hardware capabilities through tool contracts that can be consumed by MCP-compatible coding tools.

Examples:

- `device_info`
- `sensor_read`
- `display_text`
- `servo_move`
- `camera_capture`
- `wifi_scan`

### 4. Build and Delivery Layer

This layer handles:

- build
- flash
- monitor
- project scaffolding
- template selection

Supported development routes:

- `ESP-IDF`
- `Arduino`
- future optional support for `MicroPython`

### 5. Experience Layer

This layer is what users and AI tools see first.

It includes:

- examples
- docs
- prompt templates
- starter workflows
- website product selection

## Extension Model

Every platform extension should follow the same path:

1. Add hardware module definition
2. Add runtime support
3. Add MCP tool mapping
4. Add example project
5. Add docs and prompts

This is the mechanism that allows the platform to scale without becoming fragmented.

## Why This Architecture Matters

The platform should not depend on a single AI tool.

By defining a stable hardware base and an MCP-oriented tool layer, the same project can be imported into Claude, Codex, Cursor, OpenCode, or future MCP-capable tools with minimal rework.

## First Release Boundary

The first stable platform release should support:

- Core board bootstrapping
- Basic sensor read flow
- Display output flow
- Servo or motion output flow
- Standard MCP bridge tool mapping
- At least three working examples

## Platform Family Scope

Although the first connected baseline centers on `ESP32-S3`, the broader platform strategy should support three families with distinct roles:

- `ESP32` for connected and MCP-oriented development
- `STM32` for control-heavy and industrial workflows
- `Arduino-class` boards for starter, maker, and education paths

See:

- [platform-families-and-tiers.md](platform-families-and-tiers.md)
- [../hardware/memory-interface-security.md](../hardware/memory-interface-security.md)
