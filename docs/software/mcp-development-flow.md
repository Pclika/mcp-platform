# MCP Development Flow

## Goal

Allow a developer to import the platform into an MCP-compatible coding tool and immediately start useful embedded development.

## Core Idea

The repository should provide enough structure that an AI coding tool can understand:

- what hardware exists
- what the runtime can do
- what tools are exposed
- how to build and flash
- what example to start from

## Flow

### Step 1. Import Repository

Import the Pclika platform repository into:

- Claude
- Codex
- Cursor
- OpenCode
- other MCP-compatible tools

### Step 2. Read Platform Context

The tool should first read:

- `README.md`
- `AGENTS.md`
- platform docs
- relevant example folders

### Step 3. Choose a Capability Path

Examples:

- sensor
- display
- servo
- vision
- industrial communication

### Step 4. Bind Hardware and Runtime

The tool then selects:

- target board
- needed module
- runtime component
- tool surface

### Step 5. Use MCP Tool Contracts

The AI tool should work with stable capability names such as:

- `sensor_read`
- `display_text`
- `servo_move`
- `camera_capture`

### Step 6. Build and Flash

The local development flow should expose:

- build
- flash
- monitor

### Step 7. Iterate

The user or AI tool should be able to change behavior, rebuild, and retest quickly.

## First Tool Set

- `device_info`
- `led_control`
- `button_read`
- `sensor_read`
- `display_text`
- `servo_move`
- `wifi_scan`
- `serial_log_read`

## First Example Set

- `hello-mcp`
- `env-monitor`
- `servo-control`

## Success Condition

After import, a user should be able to make a few choices and reach a working path without having to design the project structure from scratch.

