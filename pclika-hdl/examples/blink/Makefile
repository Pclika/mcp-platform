# Makefile — Pclika HDL: blink example (iCE40UP5K / iCEBreaker)
#
# Targets:
#   make lint         — Verilator lint check
#   make synth        — Yosys synthesis
#   make impl         — nextpnr place & route
#   make timing       — extract timing summary
#   make pack         — icepack .asc → .bin
#   make flash        — flash bitstream to iCEBreaker
#   make sim          — run Verilator simulation
#   make clean        — remove all build artifacts
#
# Requires: yosys, nextpnr-ice40, fpga-icestorm, verilator

# ── Config ────────────────────────────────────────────────────────────────
TOP        := blink_top
DEVICE     := up5k
PACKAGE    := sg48
TARGET_MHZ := 12

RTLDIR  := rtl
TBDIR   := tb
CONDIR  := constraints
BUILDDIR := build

SOURCES   := $(RTLDIR)/$(TOP).v
PCF       := $(CONDIR)/ice40up5k.pcf

JSON      := $(BUILDDIR)/$(TOP).json
ASC       := $(BUILDDIR)/$(TOP).asc
BIN       := $(BUILDDIR)/$(TOP).bin

# ── Phony ─────────────────────────────────────────────────────────────────
.PHONY: all lint synth impl timing pack flash sim clean

all: pack

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# ── Lint ──────────────────────────────────────────────────────────────────
lint:
	verilator --lint-only -Wall -Wno-DECLFILENAME \
	    --top-module $(TOP) $(SOURCES)
	@echo "Lint: OK"

# ── Synthesis ─────────────────────────────────────────────────────────────
synth: $(JSON)

$(JSON): $(SOURCES) | $(BUILDDIR)
	yosys -p "synth_ice40 -top $(TOP) -json $@" $(SOURCES) \
	    2>&1 | tee $(BUILDDIR)/synth.log
	@echo "Synthesis: done → $(JSON)"

# ── Place & Route ─────────────────────────────────────────────────────────
impl: $(ASC)

$(ASC): $(JSON) $(PCF) | $(BUILDDIR)
	nextpnr-ice40 --$(DEVICE) --package $(PACKAGE) \
	    --json $(JSON) \
	    --pcf $(PCF) \
	    --asc $(ASC) \
	    --freq $(TARGET_MHZ) \
	    2>&1 | tee $(BUILDDIR)/impl.log
	@echo "P&R: done → $(ASC)"

# ── Timing summary ────────────────────────────────────────────────────────
timing: $(ASC)
	@grep -E "Max frequency|Critical path|Slack" $(BUILDDIR)/impl.log || \
	    echo "(no timing lines found — check build/impl.log)"

# ── Bitstream pack ────────────────────────────────────────────────────────
pack: $(BIN)

$(BIN): $(ASC) | $(BUILDDIR)
	icepack $(ASC) $(BIN)
	@echo "Pack: done → $(BIN)"

# ── Flash ─────────────────────────────────────────────────────────────────
flash: $(BIN)
	iceprog $(BIN)

# ── Simulation ────────────────────────────────────────────────────────────
sim:
	mkdir -p $(BUILDDIR)/sim
	verilator --binary --timing -Wall -Wno-fatal -Wno-DECLFILENAME \
	    --top-module tb_blink_top \
	    -GTOGGLE_CNT=6 -GCNT_WIDTH=4 \
	    --Mdir $(BUILDDIR)/sim \
	    $(RTLDIR)/$(TOP).v $(TBDIR)/tb_blink_top.v \
	    -o sim_blink
	$(BUILDDIR)/sim/sim_blink
	@echo "Simulation: done"

# ── Clean ─────────────────────────────────────────────────────────────────
clean:
	rm -rf $(BUILDDIR)
	@echo "Clean: done"
