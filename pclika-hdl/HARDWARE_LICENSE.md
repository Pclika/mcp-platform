# Hardware License — CERN Open Hardware Licence Version 2 - Strongly Reciprocal

This license applies to all **RTL hardware design files** in this repository, including:

- `hdl/rtl/*.v` — Verilog IP cores (UART, SPI, PWM, and future modules)
- `hdl/constraints/*.pcf` — FPGA pin constraint files
- `examples/*/rtl/*.v` — Example top-level designs
- `examples/*/constraints/*.pcf` — Example constraint files

---

## CERN-OHL-S v2 Summary

The CERN Open Hardware Licence Strongly Reciprocal (CERN-OHL-S) v2 is a **copyleft** hardware license. Key terms:

**You are free to:**
- Study, modify, and use these hardware designs for any purpose
- Manufacture hardware based on these designs
- Distribute the original or modified designs

**Under the following conditions:**
- **Attribution** — Retain all copyright and source notices
- **Copyleft** — If you distribute modified versions or products incorporating these designs, you must release the complete modified hardware source under CERN-OHL-S v2
- **Notice** — Include a copy of this license with any distribution

**Patent grant:**  
Contributors grant you a royalty-free, worldwide, non-exclusive patent license for patents they hold that are necessarily infringed by the covered source.

---

## Full License Text

The complete CERN-OHL-S v2 license text is available at:

**https://ohwr.org/cern_ohl_s_v2.txt**  
SPDX identifier: `CERN-OHL-S-2.0`

---

## Copyright Notice

```
Copyright 2026 Pclika (https://pclika.com)

This source describes Open Hardware and is licensed under the
CERN-OHL-S v2 or any later version.

You may redistribute and modify this source and make products using it
under the terms of the CERN-OHL-S v2 (https://ohwr.org/cern_ohl_s_v2.txt).

This source is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY,
INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A
PARTICULAR PURPOSE. Please see the CERN-OHL-S v2 for applicable conditions.

Source location: https://github.com/Pclika/pclika-hdl
```

---

## How to Apply This License to New RTL Files

Add the following header to each new `.v` or `.sv` file:

```verilog
/**
 * Copyright 2026 Pclika (https://pclika.com)
 * SPDX-License-Identifier: CERN-OHL-S-2.0
 *
 * This source describes Open Hardware and is licensed under the
 * CERN-OHL-S v2. You may redistribute and modify this source and
 * make products using it under the terms of the CERN-OHL-S v2
 * (https://ohwr.org/cern_ohl_s_v2.txt).
 */
```

---

## Why CERN-OHL-S?

The strongly-reciprocal variant ensures that improvements to the Pclika HDL IP library remain open. If you build a commercial product using this IP and release it, the modified RTL must come back to the community. The software bridge (`bridge/`) is separately licensed under Apache-2.0 and is not subject to this requirement.
