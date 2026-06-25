# Platform Families and Development Tiers

## Goal

Define a multi-platform architecture that covers ESP32, STM32, and Arduino-class hardware while serving the needs of IoT, maker, education, and industrial automation.

## Platform Family Strategy

Pclika should not treat all MCU families the same.

Each family should have a clear role.

## 1. ESP32 Family

Primary role:

- connected AI-native embedded platform

Best fit:

- IoT
- maker
- education with wireless workflows
- light industrial gateway and node prototypes

Strengths:

- Wi-Fi and BLE
- strong ecosystem
- strong AI code coverage
- suitable for MCP-oriented workflows

Recommended Pclika role:

- first-class connected platform
- primary MCP Ready Kit family
- base for sensor, display, voice, and some vision flows

## 2. STM32 Family

Primary role:

- control-heavy and industrial-oriented platform

Best fit:

- industrial automation
- deterministic control
- motor and field interface systems
- B2B prototypes and small-batch deployment platforms

Strengths:

- broad industrial ecosystem
- strong peripheral depth
- mature control and real-time tooling

Recommended Pclika role:

- industrial and control expansion family
- stronger field IO, CAN, RS485, and control workflows
- secondary family for higher-reliability edge designs

## 3. Arduino-Class Family

Primary role:

- entry-level adoption and simplified learning path

Best fit:

- maker entry
- education
- simple prototyping
- low-complexity extension demos

Strengths:

- simplicity
- large tutorial base
- low barrier to first success

Recommended Pclika role:

- onboarding family
- simplified starter examples
- teaching and workshop kits

## Development Tiers

Pclika should define development in layered tiers rather than one generic stack.

## Tier A: Starter Tier

Audience:

- beginners
- maker users
- students

Typical hardware:

- Arduino-class boards
- simple ESP32 starter kits

Capabilities:

- LED
- button
- basic sensors
- simple display
- basic servo control

Outputs:

- starter examples
- simplified MCP-compatible workflows
- quick documentation

## Tier B: Connected Builder Tier

Audience:

- serious makers
- prototyping teams
- AI tool users

Typical hardware:

- ESP32-S3
- ESP32-C3
- ESP32-C6

Capabilities:

- Wi-Fi
- BLE
- richer sensors
- display
- logging
- device configuration
- cloud-adjacent workflows

Outputs:

- Pclika MCP Ready Kit
- sensor and display workflows
- AI-assisted build and flash workflows

## Tier C: Applied Solution Tier

Audience:

- startups
- advanced builders
- technical educators

Typical hardware:

- expanded ESP32 platform
- selected STM32 variants

Capabilities:

- modular subsystem composition
- motion control
- industrial communication
- visual acquisition
- structured runtime configuration

Outputs:

- scenario kits
- applied vertical examples
- semi-custom solution builds

## Tier D: Industrial Automation Tier

Audience:

- industrial integrators
- OEM teams
- automation engineers

Typical hardware:

- STM32-centered platforms
- ESP32 plus industrial communication support where suitable

Capabilities:

- CAN
- RS485
- relay and digital IO
- field wiring support
- stronger safety and diagnostics requirements

Outputs:

- industrial kit variants
- control-heavy reference designs
- small-batch deployment path

## Domain Mapping

### IoT

Primary families:

- ESP32
- selected STM32 for field control nodes

Priority capabilities:

- wireless
- sensing
- power profiling
- logging

### Maker

Primary families:

- ESP32
- Arduino-class boards

Priority capabilities:

- easy setup
- examples
- display
- motion
- starter sensors

### Education

Primary families:

- Arduino-class boards
- ESP32 starter kits

Priority capabilities:

- predictable setup
- simple exercises
- reusable templates
- guided docs

### Industrial Automation

Primary families:

- STM32
- ESP32 where wireless gateway value is clear

Priority capabilities:

- communication buses
- control reliability
- diagnostics
- security baseline

## Platform Governance Rule

New platform additions should only happen if:

- they fit a clear family role
- they add real user value
- they can be supported across runtime, MCP tools, docs, and examples

This keeps the platform from becoming fragmented.

