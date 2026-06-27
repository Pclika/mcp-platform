"""
toolchain.py — Subprocess runner and async job manager for HDL toolchain

Wraps: Yosys, nextpnr-ice40, iceprog, Verilator, icepack
Each long-running tool (synth, impl, sim) runs in a background thread
and returns a job ID. The MCP tool polls via *_status tools.
"""

import subprocess
import threading
import uuid
import shutil
import logging
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

log = logging.getLogger("pclika_hdl.toolchain")


# ── Job ───────────────────────────────────────────────────────────────────

@dataclass
class Job:
    id: str
    status: str = "pending"   # pending | running | done | failed
    stdout: str = ""
    stderr: str = ""
    returncode: Optional[int] = None
    kind: str = ""            # synth | impl | sim | lint | flash

    def to_dict(self) -> dict:
        return {
            "job_id":     self.id,
            "status":     self.status,
            "kind":       self.kind,
            "returncode": self.returncode,
        }


# ── ToolchainRunner ───────────────────────────────────────────────────────

class ToolchainRunner:
    """
    Manages toolchain subprocess invocations and job state.

    project_root: path to the project (where hdl/ toolchain/ live)
    device:       "ice40up5k" | "ecp5" (selects tool variants)
    package:      "sg48" | "uwg30"
    freq_mhz:     default target clock frequency
    """

    def __init__(
        self,
        project_root: Path,
        device: str = "ice40up5k",
        package: str = "sg48",
        freq_mhz: float = 12.0,
    ):
        self.project_root = Path(project_root).resolve()
        self.device = device
        self.package = package
        self.freq_mhz = freq_mhz

        self._jobs: dict[str, Job] = {}
        self._last: dict[str, str] = {}   # kind → latest job id
        self._lock = threading.Lock()

        self.build_dir = self.project_root / "build"
        self.hdl_dir   = self.project_root / "hdl"
        self.build_dir.mkdir(parents=True, exist_ok=True)

    # ── Internal helpers ──────────────────────────────────────────────────

    def _new_job(self, kind: str) -> Job:
        job = Job(id=str(uuid.uuid4())[:8], kind=kind)
        with self._lock:
            self._jobs[job.id] = job
            self._last[kind] = job.id
        return job

    def _run_async(self, job: Job, cmd: list[str], cwd: Optional[Path] = None):
        """Launch cmd in a background thread; update job when done."""
        def _worker():
            job.status = "running"
            log.info("[%s] %s", job.id, " ".join(cmd))
            try:
                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    text=True,
                    cwd=str(cwd or self.project_root),
                )
                job.stdout = result.stdout
                job.stderr = result.stderr
                job.returncode = result.returncode
                job.status = "done" if result.returncode == 0 else "failed"
            except FileNotFoundError as e:
                job.stderr = f"Command not found: {e}"
                job.returncode = -1
                job.status = "failed"
            except Exception as e:
                job.stderr = str(e)
                job.returncode = -1
                job.status = "failed"
            log.info("[%s] finished: %s", job.id, job.status)

        t = threading.Thread(target=_worker, daemon=True)
        t.start()

    def _get_job(self, job_id: Optional[str], kind: str) -> Optional[Job]:
        with self._lock:
            if job_id:
                return self._jobs.get(job_id)
            last = self._last.get(kind)
            return self._jobs.get(last) if last else None

    def _tool_exists(self, name: str) -> bool:
        return shutil.which(name) is not None

    # ── Synthesis (Yosys) ─────────────────────────────────────────────────

    def synth_run(self, top_module: str, target_device: str = "ice40") -> Job:
        job = self._new_job("synth")
        json_out = self.build_dir / f"{top_module}.json"
        log_out  = self.build_dir / "synth.log"

        # Collect RTL sources
        rtl_dir = self.hdl_dir / "rtl"
        sources = list(rtl_dir.glob("**/*.v")) + list(rtl_dir.glob("**/*.sv"))
        if not sources:
            job.status = "failed"
            job.stderr = f"No RTL sources found in {rtl_dir}"
            return job

        cmd = [
            "yosys",
            "-p", f"synth_{target_device} -top {top_module} -json {json_out}",
            *[str(s) for s in sources],
        ]
        self._run_async(job, cmd)
        return job

    def synth_status(self, job_id: Optional[str] = None) -> Optional[Job]:
        return self._get_job(job_id, "synth")

    # ── Place & Route (nextpnr) ───────────────────────────────────────────

    def impl_run(
        self,
        top_module: str = "blink_top",
        seed: int = 1,
        freq_mhz: Optional[float] = None,
    ) -> Job:
        job = self._new_job("impl")
        json_in  = self.build_dir / f"{top_module}.json"
        pcf_file = self.hdl_dir / "constraints" / f"{self.device}.pcf"
        asc_out  = self.build_dir / f"{top_module}.asc"
        freq = freq_mhz or self.freq_mhz

        if not json_in.exists():
            job.status = "failed"
            job.stderr = f"{json_in} not found — run synth_run first"
            return job

        if self.device.startswith("ice40"):
            device_flag = "--up5k" if "up5k" in self.device else "--hx8k"
            cmd = [
                "nextpnr-ice40",
                device_flag,
                "--package", self.package,
                "--json",    str(json_in),
                "--pcf",     str(pcf_file),
                "--asc",     str(asc_out),
                "--freq",    str(freq),
                "--seed",    str(seed),
            ]
        else:
            job.status = "failed"
            job.stderr = f"Unsupported device for impl: {self.device}"
            return job

        self._run_async(job, cmd)
        return job

    def impl_status(self, job_id: Optional[str] = None) -> Optional[Job]:
        return self._get_job(job_id, "impl")

    # ── Bitstream pack ────────────────────────────────────────────────────

    def pack(self, top_module: str = "blink_top") -> tuple[bool, str]:
        """Run icepack to convert .asc → .bin. Returns (ok, message)."""
        asc_in  = self.build_dir / f"{top_module}.asc"
        bin_out = self.build_dir / f"{top_module}.bin"

        if not asc_in.exists():
            return False, f"{asc_in} not found — run impl_run first"

        result = subprocess.run(
            ["icepack", str(asc_in), str(bin_out)],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            return False, result.stderr
        return True, str(bin_out)

    # ── Flash (iceprog) ───────────────────────────────────────────────────

    def flash(
        self,
        top_module: str = "blink_top",
        device_path: Optional[str] = None,
        verify: bool = True,
    ) -> Job:
        job = self._new_job("flash")
        ok, msg = self.pack(top_module)
        if not ok:
            job.status = "failed"
            job.stderr = msg
            return job

        bin_file = self.build_dir / f"{top_module}.bin"
        cmd = ["iceprog"]
        if not verify:
            cmd.append("-n")
        if device_path:
            cmd += ["-d", device_path]
        cmd.append(str(bin_file))

        self._run_async(job, cmd)
        return job

    # ── Lint (Verilator) ─────────────────────────────────────────────────

    def lint(self, top_module: Optional[str] = None, strict: bool = False) -> Job:
        job = self._new_job("lint")
        rtl_dir = self.hdl_dir / "rtl"
        sources = list(rtl_dir.glob("**/*.v")) + list(rtl_dir.glob("**/*.sv"))

        if not sources:
            job.status = "failed"
            job.stderr = f"No RTL sources found in {rtl_dir}"
            return job

        cmd = ["verilator", "--lint-only", "-Wall"]
        if strict:
            cmd.append("-Wno-fatal")   # keep going but capture all
        else:
            cmd.append("-Wno-fatal")

        if top_module:
            cmd += ["--top-module", top_module]

        cmd += [str(s) for s in sources]

        # Lint is fast — run synchronously
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=str(self.project_root))
        job.stdout = result.stdout
        job.stderr = result.stderr
        job.returncode = result.returncode
        job.status = "done" if result.returncode == 0 else "failed"
        return job

    # ── Simulation (Verilator) ────────────────────────────────────────────

    def sim_run(
        self,
        testbench: str,
        timeout_cycles: int = 1_000_000,
        save_waveform: bool = False,
    ) -> Job:
        job = self._new_job("sim")
        tb_dir  = self.hdl_dir / "tb"
        rtl_dir = self.hdl_dir / "rtl"
        sim_dir = self.build_dir / "sim"
        sim_dir.mkdir(parents=True, exist_ok=True)

        tb_file = tb_dir / f"{testbench}.v"
        if not tb_file.exists():
            # Try examples
            for tb_path in self.project_root.rglob(f"{testbench}.v"):
                tb_file = tb_path
                break

        if not tb_file.exists():
            job.status = "failed"
            job.stderr = f"Testbench {testbench}.v not found"
            return job

        # Find RTL sources (all .v in rtl/)
        rtl_sources = list(rtl_dir.glob("**/*.v")) if rtl_dir.exists() else []
        # Also scan example rtl/ dirs
        for rtl_v in tb_file.parent.parent.rglob("rtl/*.v"):
            if rtl_v not in rtl_sources:
                rtl_sources.append(rtl_v)

        cmd = [
            "verilator", "--binary", "--timing",
            "-Wall", "-Wno-fatal", "-Wno-DECLFILENAME",
            "--top-module", testbench,
            "--Mdir",  str(sim_dir),
            "-o", f"sim_{testbench}",
        ]
        if save_waveform:
            cmd += ["--trace", "--trace-depth", "5"]

        cmd += [str(tb_file)] + [str(s) for s in rtl_sources]
        self._run_async(job, cmd)
        return job

    def sim_result(self, job_id: Optional[str] = None) -> Optional[Job]:
        return self._get_job(job_id, "sim")

    # ── Constraint validate ───────────────────────────────────────────────

    def constraint_validate(self, constraint_file: Optional[str] = None) -> dict:
        """
        Basic .pcf validation: check file exists, parse set_io entries,
        return port list. Full DRC requires nextpnr.
        """
        if constraint_file:
            pcf = Path(constraint_file)
        else:
            pcf = self.hdl_dir / "constraints" / f"{self.device}.pcf"

        if not pcf.exists():
            return {"valid": False, "error": f"Constraint file not found: {pcf}"}

        pins = []
        errors = []
        with open(pcf) as f:
            for lineno, line in enumerate(f, 1):
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                parts = line.split()
                if parts[0] == "set_io":
                    if len(parts) < 3:
                        errors.append(f"Line {lineno}: malformed set_io")
                    else:
                        pins.append({"port": parts[1], "pin": parts[2]})
                else:
                    errors.append(f"Line {lineno}: unknown directive '{parts[0]}'")

        return {
            "valid":            len(errors) == 0,
            "constraint_file":  str(pcf),
            "port_count":       len(pins),
            "ports":            pins,
            "errors":           errors,
        }

    # ── Device info ───────────────────────────────────────────────────────

    def device_info(self) -> dict:
        device_profiles = {
            "ice40up5k": {
                "device":       "iCE40UP5K",
                "family":       "Lattice iCE40 UltraPlus",
                "package":      self.package.upper(),
                "speed_grade":  "-8",
                "resources": {
                    "LUT4":  {"total": 5280},
                    "FF":    {"total": 5280},
                    "BRAM":  {"total": 30, "unit": "4Kbit blocks"},
                    "SPRAM": {"total": 4,  "unit": "64Kbit blocks"},
                    "DSP":   {"total": 8,  "unit": "16x16 mul"},
                    "PLL":   {"total": 1},
                    "IO":    {"total": 39},
                },
                "max_freq_mhz": 100,
                "toolchain": ["yosys", "nextpnr-ice40", "iceprog"],
                "constraint_format": ".pcf",
            },
            "ecp5": {
                "device":       "LFE5U-25F",
                "family":       "Lattice ECP5",
                "package":      "CABGA256",
                "speed_grade":  "-6",
                "resources": {
                    "LUT4":  {"total": 24288},
                    "FF":    {"total": 24288},
                    "BRAM":  {"total": 56, "unit": "18Kbit blocks"},
                    "DSP":   {"total": 28, "unit": "18x18 mul"},
                    "PLL":   {"total": 2},
                },
                "max_freq_mhz": 200,
                "toolchain": ["yosys", "nextpnr-ecp5", "openFPGALoader"],
                "constraint_format": ".lpf",
            },
        }

        profile = device_profiles.get(self.device.lower(), {})
        return {
            "configured_device": self.device,
            "target_freq_mhz":   self.freq_mhz,
            "project_root":      str(self.project_root),
            "seal":              "PCK-MMXXVI-9198580D",
            **profile,
            "toolchain_available": {
                t: self._tool_exists(t)
                for t in profile.get("toolchain", [])
            },
        }
