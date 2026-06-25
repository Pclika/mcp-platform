# Hardware Extension Plan

## Goal

Build a clear extension roadmap that expands the platform without breaking the simplicity of the base product.

## Extension Families

### 1. Vision

Purpose:

- image capture
- basic visual inspection
- AI-assisted camera workflows

Priority modules:

- camera module
- lighting accessory
- simple image trigger input

Suggested first use cases:

- image snapshot demo
- visual monitoring
- object presence check

### 2. Sensor

Purpose:

- environmental sensing
- physical world data collection
- data logging

Priority modules:

- temperature and humidity
- light
- IMU
- distance
- air quality

Suggested first use cases:

- environmental monitor
- motion tracker
- device telemetry node

### 3. Motion and Servo

Purpose:

- movement
- positioning
- robotics control

Priority modules:

- single servo output module
- multi-servo control module
- DC motor driver
- stepper interface

Suggested first use cases:

- pan-tilt control
- actuator trigger
- simple robot arm control

### 4. Display and HMI

Purpose:

- direct user feedback
- local interface
- demo visualization

Priority modules:

- OLED
- small TFT
- buzzer
- status LED array

Suggested first use cases:

- local status dashboard
- sensor display
- AI command confirmation

### 5. Industrial IO

Purpose:

- bridge the maker platform toward B2B and field scenarios

Priority modules:

- RS485
- CAN
- relay
- digital input and output

Suggested first use cases:

- field data collection
- PLC-adjacent control
- simple gateway demos

### 6. Power and Battery

Purpose:

- mobility
- power experiments
- low-power workflows

Priority modules:

- battery pack
- charging module
- low-power measurement accessory

Suggested first use cases:

- battery node demo
- power profiling
- portable logging

## Recommended Release Order

Phase 1:

- sensor
- display
- servo

Phase 2:

- vision
- industrial IO

Phase 3:

- power and battery
- specialized vertical modules

## Module Acceptance Criteria

Each module should have:

- hardware definition
- driver plan
- MCP tool mapping
- at least one example workflow
- website product description
- starter documentation

## Why This Matters

This extension model turns the platform into a capability engine.

Users should be able to choose a few options such as:

- sensing
- vision
- motion
- communication

Then immediately receive the right hardware kit, runtime support, example project, and MCP-ready development context.

