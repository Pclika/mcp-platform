# Memory, Interface, Wireless, and Security Standard

## Goal

Define the platform-level rules for memory management, external interfaces, wireless control, and security so that the Pclika hardware foundation stays stable, scalable, and safe across product variants.

## Why This Standard Matters

For Pclika, the most important hardware risks are not only feature gaps. They are:

- unstable memory behavior
- uncontrolled pin and bus growth
- weak wireless lifecycle management
- poor key and credential handling
- inconsistent behavior across board families

This document defines the baseline that every Pclika platform board or module should follow.

## 1. Memory Management Standard

### 1.1 Design Principles

- avoid hidden dynamic allocation growth
- define predictable boot-time memory layout
- separate core runtime memory from extension memory
- keep logs, buffers, and streaming workloads bounded
- support graceful degradation when memory is constrained

### 1.2 Memory Zones

Every supported platform should conceptually separate memory into:

- boot and system region
- core runtime region
- communication buffers
- module driver buffers
- temporary task workspace
- logging and diagnostics buffers
- persistent configuration storage

### 1.3 Runtime Memory Rules

- core system tasks should reserve fixed baseline memory
- sensor and control paths should prefer static or pool-based buffers
- vision and streaming paths should use explicit feature gating
- avoid unbounded string building in firmware
- monitor heap usage before and after module initialization

### 1.4 Storage Rules

- separate firmware image storage from user configuration storage
- reserve space for version metadata and board identity
- reserve space for secure credentials and provisioning data
- define a migration strategy for persistent config schema changes

### 1.5 Platform Guidance

#### ESP32 Family

- support internal RAM planning and optional PSRAM-aware feature tiers
- use PSRAM only for workloads that tolerate its latency profile
- keep control-critical tasks in predictable internal memory where possible

#### STM32 Family

- clearly define SRAM bank use across runtime domains
- separate interrupt-critical tasks from larger application buffers
- use external memory only for explicitly higher-tier variants

#### Arduino-Class Boards

- keep feature sets sharply bounded by RAM size
- avoid complex concurrent workloads on low-memory boards
- reserve Arduino-class platforms for starter, education, and simple control paths

## 2. External Interface Standard

## 2.1 Interface Design Principles

- every interface must be named and documented
- avoid ad hoc pin remapping for modules
- keep expansion headers stable across platform revisions
- prefer capability families over board-specific wiring assumptions

## 2.2 Required Base Interfaces

Every primary platform should define standard access for:

- GPIO
- I2C
- SPI
- UART
- PWM
- ADC
- power input
- debug and flashing

## 2.3 Expansion Header Model

Recommended standard module families:

- sensor header
- display header
- motion header
- industrial communication header
- camera or vision header
- power and battery header

Each family should define:

- signal type
- supply rail
- voltage limits
- hot-plug expectations
- recommended cable or connector type

## 2.4 Compatibility Rules

- modules should declare supported platform families
- modules should declare required buses and power profile
- if a module needs a special signal path, it should be isolated to its module family

## 3. Wireless Control Standard

## 3.1 Scope

This standard covers:

- Wi-Fi
- Bluetooth LE
- future optional field wireless such as Thread or similar variants

## 3.2 Wireless Principles

- wireless should be treated as a managed subsystem, not a one-off feature
- credentials should never be hardcoded in shipping examples
- connection state should be observable through runtime and tool interfaces
- provisioning should be explicit and recoverable

## 3.3 Wi-Fi Rules

- support scan, connect, disconnect, status, and recovery flows
- store credentials in protected persistent storage
- allow safe reset of network configuration
- surface signal and connection diagnostics through runtime APIs

## 3.4 Bluetooth Rules

- clearly separate provisioning, telemetry, and control roles
- avoid ambiguous pairing behavior
- document device identity and advertising rules
- provide an explicit disable path for unused Bluetooth features

## 3.5 Wireless Diagnostics

The runtime and MCP layer should expose:

- wireless capability flags
- current connection state
- scan status
- signal quality
- provisioning state
- last known error class

## 4. Security Standard

## 4.1 Security Principles

- minimum secrets on device
- explicit device identity
- protected update path
- secure configuration storage
- least-privilege MCP tool exposure

## 4.2 Device Identity

Every board should support:

- unique board identifier
- firmware version reporting
- hardware revision reporting
- capability declaration

## 4.3 Credential Handling

- credentials must be stored in protected non-volatile storage
- no production credentials in example repositories
- support credential reset and reprovisioning
- separate development and production provisioning flows

## 4.4 Boot and Firmware Integrity

Platform families should support the strongest practical integrity path available to them.

Examples:

- signed firmware where feasible
- verified boot where supported
- versioned upgrade flow
- rollback-safe update strategy for connected variants

## 4.5 MCP and Tool Surface Security

- expose only necessary actions through tool schemas
- dangerous actions must require explicit intent and confirmation semantics
- read versus write capabilities should be distinguishable
- tool contracts must avoid leaking secrets into logs

## 4.6 Field Security Baseline

For connected and industrial-oriented variants, the recommended baseline includes:

- authenticated provisioning flow
- secure credential storage
- controlled update path
- runtime diagnostics
- event logging for connection and configuration changes

## 5. Platform Review Checklist

Every board or module should be reviewed against:

- memory budget
- interface map
- power profile
- wireless profile
- credential model
- firmware update model
- MCP tool exposure

## 6. Minimum First-Generation Baseline

For the first-generation Pclika platform:

- ESP32-S3 should be the primary connected platform
- STM32 should be the primary control and industrial expansion family
- Arduino-class boards should remain in bounded education and maker roles
- memory, interface, wireless, and security behavior must be documented before public module expansion

