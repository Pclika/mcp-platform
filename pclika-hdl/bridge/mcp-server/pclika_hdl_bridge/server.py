"""
server.py — MCP STDIO server for Pclika HDL Platform

JSON-RPC 2.0 over STDIO. Same protocol as mcp-platform bridge but wraps
HDL toolchain tools instead of serial transport.

Supported methods:
  initialize, tools/list, tools/call, ping
"""

import sys
import json
import logging
from pathlib import Path

from . import __version__, SEAL, SERVER_NAME
from .toolchain import ToolchainRunner
from . import parsers

log = logging.getLogger("pclika_hdl.server")


# ── Tool registry ─────────────────────────────────────────────────────────

TOOLS = [
    {
        "name": "device_info",
        "description": "Return the target FPGA device profile: family, package, resource totals, toolchain availability.",
        "inputSchema": {"type": "object", "properties": {}, "required": []},
    },
    {
        "name": "synth_run",
        "description": "Trigger Yosys synthesis. Returns a job_id. Poll with synth_status.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "top_module":    {"type": "string", "description": "Top-level module name."},
                "target_device": {"type": "string", "default": "ice40", "description": "ice40 or ecp5."},
            },
            "required": ["top_module"],
        },
    },
    {
        "name": "synth_status",
        "description": "Return the result of the most recent synthesis run.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "job_id": {"type": "string", "description": "Job ID from synth_run (optional)."},
            },
            "required": [],
        },
    },
    {
        "name": "impl_run",
        "description": "Trigger nextpnr place-and-route. Returns a job_id. Poll with impl_status.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "seed":     {"type": "integer", "default": 1},
                "freq_mhz": {"type": "number", "description": "Target clock MHz."},
            },
            "required": [],
        },
    },
    {
        "name": "impl_status",
        "description": "Return the result of the most recent place-and-route run.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "job_id": {"type": "string"},
            },
            "required": [],
        },
    },
    {
        "name": "timing_report",
        "description": "Return timing analysis from the last P&R run: worst slack, max frequency, violated paths.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "verbose": {"type": "boolean", "default": False},
            },
            "required": [],
        },
    },
    {
        "name": "resource_usage",
        "description": "Return FPGA resource utilization from the last P&R run: LUT, FF, BRAM, DSP — used/total/%.",
        "inputSchema": {"type": "object", "properties": {}, "required": []},
    },
    {
        "name": "lint_report",
        "description": "Run Verilator lint on all RTL sources. Returns warnings and errors by file and line.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "top_module": {"type": "string"},
                "strict":     {"type": "boolean", "default": False},
            },
            "required": [],
        },
    },
    {
        "name": "sim_run",
        "description": "Trigger Verilator simulation for a testbench. Returns a job_id.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "testbench":      {"type": "string", "description": "Testbench module name (must exist in hdl/tb/)."},
                "timeout_cycles": {"type": "integer", "default": 1000000},
                "save_waveform":  {"type": "boolean", "default": False},
            },
            "required": ["testbench"],
        },
    },
    {
        "name": "sim_result",
        "description": "Return the result of the most recent simulation: pass/fail, assertion count, log tail.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "job_id": {"type": "string"},
            },
            "required": [],
        },
    },
    {
        "name": "constraint_validate",
        "description": "Validate the device constraint file (.pcf/.lpf/.xdc) and return port list.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "constraint_file": {"type": "string"},
            },
            "required": [],
        },
    },
    {
        "name": "bitstream_flash",
        "description": "Flash the latest bitstream to the connected FPGA via iceprog or openFPGALoader.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "device_path": {"type": "string"},
                "verify":      {"type": "boolean", "default": True},
            },
            "required": [],
        },
    },
    {
        "name": "waveform_export",
        "description": "Export a portion of the last simulation waveform as a text signal summary.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "signals":     {"type": "array", "items": {"type": "string"}},
                "start_cycle": {"type": "integer"},
                "end_cycle":   {"type": "integer"},
            },
            "required": [],
        },
    },
]


# ── MCP Server ────────────────────────────────────────────────────────────

class HDLMCPServer:

    def __init__(self, runner: ToolchainRunner):
        self.runner = runner
        self._initialized = False

    # ── MCP response helpers ──────────────────────────────────────────────

    @staticmethod
    def _ok(req_id, data: dict):
        return {
            "jsonrpc": "2.0",
            "id":      req_id,
            "result": {
                "content": [{"type": "text", "text": json.dumps(data, indent=2)}],
                "isError": False,
            },
        }

    @staticmethod
    def _err(req_id, message: str):
        return {
            "jsonrpc": "2.0",
            "id":      req_id,
            "result": {
                "content": [{"type": "text", "text": json.dumps({"error": message})}],
                "isError": True,
            },
        }

    # ── Tool dispatch ─────────────────────────────────────────────────────

    def _handle_tool(self, name: str, params: dict) -> dict:
        r = self.runner

        if name == "device_info":
            return r.device_info()

        elif name == "synth_run":
            top = params.get("top_module")
            if not top:
                return {"error": "top_module is required"}
            device = params.get("target_device", "ice40")
            job = r.synth_run(top, device)
            return {**job.to_dict(), "message": "Synthesis started. Poll with synth_status."}

        elif name == "synth_status":
            job = r.synth_status(params.get("job_id"))
            if not job:
                return {"error": "No synthesis job found. Run synth_run first."}
            result = {**job.to_dict()}
            if job.status in ("done", "failed"):
                result["parsed"] = parsers.parse_synth_log(job.stdout, job.stderr)
            return result

        elif name == "impl_run":
            job = r.impl_run(
                seed=params.get("seed", 1),
                freq_mhz=params.get("freq_mhz"),
            )
            return {**job.to_dict(), "message": "P&R started. Poll with impl_status."}

        elif name == "impl_status":
            job = r.impl_status(params.get("job_id"))
            if not job:
                return {"error": "No impl job found. Run impl_run first."}
            result = {**job.to_dict()}
            if job.status in ("done", "failed"):
                result["parsed"] = parsers.parse_impl_log(job.stdout, job.stderr)
            return result

        elif name == "timing_report":
            job = r.impl_status()
            if not job or job.status not in ("done", "failed"):
                return {"error": "No completed P&R run. Run impl_run first."}
            parsed = parsers.parse_impl_log(job.stdout, job.stderr)
            result = parsed.get("timing", {})
            if params.get("verbose"):
                result["log_tail"] = (job.stdout + job.stderr).splitlines()[-30:]
            return result

        elif name == "resource_usage":
            job = r.impl_status()
            if not job or job.status not in ("done", "failed"):
                return {"error": "No completed P&R run. Run impl_run first."}
            parsed = parsers.parse_impl_log(job.stdout, job.stderr)
            return parsed.get("resources", {})

        elif name == "lint_report":
            job = r.lint(
                top_module=params.get("top_module"),
                strict=params.get("strict", False),
            )
            result = {**job.to_dict()}
            result["parsed"] = parsers.parse_lint_output(job.stdout, job.stderr)
            return result

        elif name == "sim_run":
            tb = params.get("testbench")
            if not tb:
                return {"error": "testbench is required"}
            job = r.sim_run(
                testbench=tb,
                timeout_cycles=params.get("timeout_cycles", 1_000_000),
                save_waveform=params.get("save_waveform", False),
            )
            return {**job.to_dict(), "message": "Simulation started. Poll with sim_result."}

        elif name == "sim_result":
            job = r.sim_result(params.get("job_id"))
            if not job:
                return {"error": "No simulation job found. Run sim_run first."}
            result = {**job.to_dict()}
            if job.status in ("done", "failed"):
                result["parsed"] = parsers.parse_sim_output(job.stdout, job.stderr)
            return result

        elif name == "constraint_validate":
            return r.constraint_validate(params.get("constraint_file"))

        elif name == "bitstream_flash":
            job = r.flash(
                device_path=params.get("device_path"),
                verify=params.get("verify", True),
            )
            return {**job.to_dict(), "message": "Flash started."}

        elif name == "waveform_export":
            # VCD parsing is complex — return a stub with guidance
            return {
                "note": "Waveform export requires simulation run with save_waveform=true.",
                "signals": params.get("signals", []),
                "hint": "Check build/sim/*.vcd with GTKWave for full waveform viewing.",
            }

        else:
            return {"error": f"Unknown tool: {name}"}

    # ── Main loop ─────────────────────────────────────────────────────────

    def _dispatch(self, req: dict) -> dict:
        method = req.get("method", "")
        req_id = req.get("id")
        params = req.get("params", {})

        if method == "ping":
            return {"jsonrpc": "2.0", "id": req_id, "result": {}}

        if method == "initialize":
            self._initialized = True
            return {
                "jsonrpc": "2.0",
                "id":      req_id,
                "result": {
                    "protocolVersion": "2024-11-05",
                    "capabilities":    {"tools": {}},
                    "serverInfo": {
                        "name":    SERVER_NAME,
                        "version": __version__,
                        "seal":    SEAL,
                    },
                },
            }

        if method == "tools/list":
            return {
                "jsonrpc": "2.0",
                "id":      req_id,
                "result":  {"tools": TOOLS},
            }

        if method == "tools/call":
            tool_name = params.get("name", "")
            tool_args = params.get("arguments", {})
            try:
                data = self._handle_tool(tool_name, tool_args)
                return self._ok(req_id, data)
            except Exception as e:
                log.exception("Tool %s failed", tool_name)
                return self._err(req_id, str(e))

        if method == "notifications/initialized":
            return None   # no response for notifications

        return {
            "jsonrpc": "2.0",
            "id":      req_id,
            "error": {"code": -32601, "message": f"Method not found: {method}"},
        }

    def run(self):
        log.info("%s v%s ready (STDIO)", SERVER_NAME, __version__)
        log.info("Seal: %s", SEAL)
        log.info("Project: %s", self.runner.project_root)
        log.info("Device:  %s", self.runner.device)

        for line in sys.stdin:
            line = line.strip()
            if not line:
                continue
            try:
                req = json.loads(line)
            except json.JSONDecodeError as e:
                resp = {"jsonrpc": "2.0", "id": None,
                        "error": {"code": -32700, "message": f"Parse error: {e}"}}
                print(json.dumps(resp), flush=True)
                continue

            resp = self._dispatch(req)
            if resp is not None:
                print(json.dumps(resp), flush=True)
