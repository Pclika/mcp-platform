"""
parsers.py — Parse Yosys and nextpnr text output into structured data

Yosys and nextpnr write human-readable logs. We extract the numbers
AI tools need: resource counts, timing slack, warnings, errors.
"""

import re
from typing import Optional


# ── Yosys synthesis output parser ────────────────────────────────────────

def parse_synth_log(stdout: str, stderr: str) -> dict:
    """
    Parse combined Yosys output.
    Returns: {
        cells_total, wires_total, warnings, errors,
        modules, resource_estimate
    }
    """
    text  = stdout + "\n" + stderr
    lines = text.splitlines()

    cells_total = None
    wires_total = None
    modules     = []
    warnings    = []
    errors      = []
    resource_estimate = {}

    for line in lines:
        # Cell count: "   Number of cells:              42"
        m = re.search(r"Number of cells:\s+(\d+)", line)
        if m:
            cells_total = int(m.group(1))

        # Wire count: "   Number of wires:              38"
        m = re.search(r"Number of wires:\s+(\d+)", line)
        if m:
            wires_total = int(m.group(1))

        # Module: "   Selecting module blink_top."
        m = re.search(r"Selecting module (\w+)\.", line)
        if m and m.group(1) not in modules:
            modules.append(m.group(1))

        # Resource cells (after synthesis): "SB_LUT4          12"
        m = re.match(r"\s+(SB_LUT4|SB_FF|SB_RAM|ICESTORM_LC|TRELLIS_FF)\s+(\d+)", line)
        if m:
            resource_estimate[m.group(1)] = int(m.group(2))

        # Warnings
        if "Warning:" in line or "warning:" in line.lower():
            warnings.append(line.strip())

        # Errors
        if "Error:" in line or "error:" in line.lower():
            errors.append(line.strip())

    return {
        "cells_total":        cells_total,
        "wires_total":        wires_total,
        "modules":            modules,
        "resource_estimate":  resource_estimate,
        "warning_count":      len(warnings),
        "error_count":        len(errors),
        "warnings":           warnings[:10],   # first 10
        "errors":             errors[:10],
    }


# ── nextpnr place & route output parser ──────────────────────────────────

def parse_impl_log(stdout: str, stderr: str) -> dict:
    """
    Parse nextpnr output.
    Returns: {
        routing_success, wire_usage_pct, logic_util_pct,
        timing: { worst_slack_ns, max_freq_mhz, violated_paths },
        resources: { LUT4, FF, BRAM, ... },
        warnings, errors
    }
    """
    text  = stdout + "\n" + stderr
    lines = text.splitlines()

    routing_success  = None
    wire_usage_pct   = None
    logic_util_pct   = None
    worst_slack_ns   = None
    max_freq_mhz     = None
    violated_paths   = 0
    resources        = {}
    warnings         = []
    errors           = []

    for line in lines:
        # Routing success/fail
        if "Routing complete." in line:
            routing_success = True
        if "Routing failed." in line or "ERROR" in line:
            routing_success = False

        # Wire usage: "Wire usage: 1234/5280 (23%)"
        m = re.search(r"Wire usage:\s*(\d+)/(\d+)\s*\((\d+)%\)", line)
        if m:
            wire_usage_pct = int(m.group(3))

        # Logic util: "Logic util: 12/5280 (0%)" or "ICESTORM_LC"
        m = re.search(r"ICESTORM_LC\s+(\d+)/(\d+)\s+(\d+)%", line)
        if m:
            resources["LUT4"] = {
                "used": int(m.group(1)),
                "total": int(m.group(2)),
                "pct": int(m.group(3)),
            }
            logic_util_pct = int(m.group(3))

        # FF: "SB_FF          26/5280     0%"
        m = re.search(r"SB_FF\s+(\d+)/(\d+)\s+(\d+)%", line)
        if m:
            resources["FF"] = {
                "used": int(m.group(1)),
                "total": int(m.group(2)),
                "pct": int(m.group(3)),
            }

        # BRAM: "ICESTORM_RAM   0/30        0%"
        m = re.search(r"ICESTORM_RAM\s+(\d+)/(\d+)\s+(\d+)%", line)
        if m:
            resources["BRAM"] = {
                "used": int(m.group(1)),
                "total": int(m.group(2)),
                "pct": int(m.group(3)),
            }

        # DSP: "SB_MAC16       0/8         0%"
        m = re.search(r"SB_MAC16\s+(\d+)/(\d+)\s+(\d+)%", line)
        if m:
            resources["DSP"] = {
                "used": int(m.group(1)),
                "total": int(m.group(2)),
                "pct": int(m.group(3)),
            }

        # Max frequency: "Max frequency for clock 'clk': 87.23 MHz"
        m = re.search(r"Max frequency.*?:\s*([\d.]+)\s*MHz", line)
        if m:
            max_freq_mhz = float(m.group(1))

        # Worst slack: "Worst slack: -0.12ns"
        m = re.search(r"Worst slack:\s*([-\d.]+)\s*ns", line)
        if m:
            worst_slack_ns = float(m.group(1))

        # Violated paths
        m = re.search(r"(\d+)\s+path.*violated", line, re.IGNORECASE)
        if m:
            violated_paths = int(m.group(1))

        # Warnings / errors
        if "Warning:" in line:
            warnings.append(line.strip())
        if "Error:" in line or "ERROR:" in line:
            errors.append(line.strip())
            routing_success = False

    timing = {
        "worst_slack_ns":  worst_slack_ns,
        "max_freq_mhz":    max_freq_mhz,
        "violated_paths":  violated_paths,
        "timing_clean":    (worst_slack_ns is not None and worst_slack_ns >= 0),
    }

    return {
        "routing_success": routing_success,
        "wire_usage_pct":  wire_usage_pct,
        "logic_util_pct":  logic_util_pct,
        "timing":          timing,
        "resources":       resources,
        "warning_count":   len(warnings),
        "error_count":     len(errors),
        "warnings":        warnings[:10],
        "errors":          errors[:10],
    }


# ── Verilator lint parser ─────────────────────────────────────────────────

def parse_lint_output(stdout: str, stderr: str) -> dict:
    """
    Parse verilator --lint-only output.
    Returns: { warnings, errors, warning_count, error_count }
    """
    text  = stdout + "\n" + stderr
    lines = text.splitlines()

    warnings = []
    errors   = []

    for line in lines:
        # Verilator: %Warning-UNUSED: file.v:10:5: message
        m = re.match(r"%(Warning|Error)[^:]*:\s*(.+\.v):(\d+):(\d+):\s*(.+)", line)
        if m:
            entry = {
                "severity": m.group(1),
                "file":     m.group(2),
                "line":     int(m.group(3)),
                "col":      int(m.group(4)),
                "message":  m.group(5).strip(),
            }
            if m.group(1) == "Warning":
                warnings.append(entry)
            else:
                errors.append(entry)
        # Plain error line
        elif re.match(r"%Error", line):
            errors.append({"severity": "Error", "message": line.strip()})

    return {
        "warning_count": len(warnings),
        "error_count":   len(errors),
        "warnings":      warnings,
        "errors":        errors,
        "lint_clean":    len(errors) == 0,
    }


# ── Simulation output parser ──────────────────────────────────────────────

def parse_sim_output(stdout: str, stderr: str) -> dict:
    """
    Parse Verilator simulation output (from testbench $display statements).
    Returns: { passed, assertion_count, log_tail }
    """
    text  = stdout + "\n" + stderr
    lines = [l for l in text.splitlines() if l.strip()]

    passed          = None
    assertion_count = 0
    fail_count      = 0

    for line in lines:
        if "PASS:" in line:
            passed = True
            m = re.search(r"(\d+)\s+assertion", line)
            if m:
                assertion_count = int(m.group(1))
        if "FAIL:" in line:
            passed = False
            fail_count += 1
        if "TIMEOUT" in line:
            passed = False

    return {
        "passed":          passed,
        "assertion_count": assertion_count,
        "fail_count":      fail_count,
        "log_tail":        lines[-50:],   # last 50 lines
    }
