# Base Hardware Platform

## Goal

Define a stable first-generation hardware platform that supports rapid AI-assisted embedded development and future module expansion.

## Baseboard Priority

The first-generation platform should prioritize:

- high ecosystem maturity
- stable wireless support
- good AI code coverage
- low setup friction
- enough performance for sensing, display, and edge workflows

## Recommended Base MCU

Primary choice:

- `ESP32-S3`

Why:

- strong documentation and examples
- good fit for Wi-Fi and BLE workflows
- mature toolchains
- enough performance for sensor, display, voice, and basic vision extensions

## Required Baseboard Capabilities

### Core

- ESP32-S3 MCU
- USB-C power and data
- onboard LED
- onboard button
- boot and reset access

### IO

- GPIO breakout
- I2C header
- SPI header
- UART header
- PWM-capable pins
- ADC-capable pins

### Expansion

- standard sensor header
- standard display header
- standard motion header
- standard industrial header

### Power

- USB 5V input
- 3.3V regulated rail
- optional battery connector
- protected power path for future field use

### Debug

- serial logging
- flashing access
- optional external debug accessory support

## Base Functional Definition

The baseboard should support these first-class functions:

- system identity and version reporting
- digital input and output
- analog reading
- I2C peripheral access
- SPI peripheral access
- display output
- basic wireless scan and configuration
- servo or PWM output
- sensor polling

## Mechanical Standard

The board should be designed for repeatable assembly and expansion:

- mounting holes
- stable module orientation
- clear pin labeling
- optional enclosure compatibility
- optional DIN mounting path for industrial variant

## Product Variants

### Basic

Best for:

- makers
- early developers
- AI workflow demos

### Pro

Adds:

- richer modules
- display path
- more complete examples

### Industrial

Adds:

- RS485
- CAN
- field wiring support
- DIN compatibility

## Hardware Rules for Expansion

Every new hardware module should:

- fit the platform interface model
- expose a clean driver boundary
- avoid one-off pin chaos
- support documentation and examples
- be useful to both users and AI-assisted workflows

## Reliability and Control Baseline

The base platform must also be designed around stronger infrastructure concerns:

- explicit memory budgeting
- stable interface families
- managed Wi-Fi and Bluetooth lifecycle
- secure credential handling
- diagnostics visibility for runtime and MCP tools

For the detailed platform rules, see:

- [memory-interface-security.md](memory-interface-security.md)
- [../architecture/platform-families-and-tiers.md](../architecture/platform-families-and-tiers.md)
