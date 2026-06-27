# Pclika HDL Platform — Standard Makefile
#
# Use this Makefile from any example directory, or copy it to your project root.
# All Pclika HDL examples include this pattern.
#
# Standard targets:
#   make lint            → Verilator lint check
#   make synth           → Yosys synthesis
#   make impl            → nextpnr place & route
#   make timing          → print timing summary
#   make pack            → icepack .asc → .bin
#   make flash           → flash bitstream to device
#   make sim             → run simulation (set TARGET=tb_<name>)
#   make clean           → remove build/
#   make info            → print device and tool versions
#
# Configuration:
#   Override variables on the command line:
#   make synth TOP=my_top
#   make impl  SEED=42 FREQ=50
#   make sim   TARGET=tb_my_module

# ── Project config ────────────────────────────────────────────────────────
TOP     ?= $(shell basename $(CURDIR))_top
DEVICE  ?= up5k
PKG     ?= sg48
FREQ    ?= 12
SEED    ?= 1

# Directories
RTLDIR  ?= rtl
TBDIR   ?= tb
CONDIR  ?= constraints
BUILD   ?= build

# Source files (all .v and .sv in rtl/)
SRCS    := $(wildcard $(RTLDIR)/*.v) $(wildcard $(RTLDIR)/*.sv)
PCF     ?= $(CONDIR)/ice40up5k.pcf

# Build artifacts
JSON    := $(BUILD)/$(TOP).json
ASC     := $(BUILD)/$(TOP).asc
BIN     := $(BUILD)/$(TOP).bin

# Tools
YOSYS        ?= yosys
NEXTPNR      ?= nextpnr-ice40
ICEPACK      ?= icepack
ICEPROG      ?= iceprog
VERILATOR    ?= verilator

# ── Phony targets ─────────────────────────────────────────────────────────
.PHONY: all lint synth impl timing pack flash sim clean info

all: pack

$(BUILD):
	@mkdir -p $(BUILD)

# ── Lint ──────────────────────────────────────────────────────────────────
lint:
	@echo "[lint] Running Verilator lint on $(TOP)..."
	$(VERILATOR) --lint-only -Wall -Wno-DECLFILENAME \
	    --top-module $(TOP) $(SRCS)
	@echo "[lint] OK"

# ── Synthesis ─────────────────────────────────────────────────────────────
synth: $(JSON)

$(JSON): $(SRCS) | $(BUILD)
	@echo "[synth] Yosys → $(JSON)"
	$(YOSYS) -p "synth_ice40 -top $(TOP) -json $@" $(SRCS) \
	    2>&1 | tee $(BUILD)/synth.log
	@echo "[synth] Done. See $(BUILD)/synth.log"

# ── Place & Route ─────────────────────────────────────────────────────────
impl: $(ASC)

$(ASC): $(JSON) $(PCF) | $(BUILD)
	@echo "[impl] nextpnr → $(ASC) (seed=$(SEED), freq=$(FREQ)MHz)"
	$(NEXTPNR) --$(DEVICE) --package $(PKG) \
	    --json $(JSON) \
	    --pcf $(PCF) \
	    --asc $(ASC) \
	    --freq $(FREQ) \
	    --seed $(SEED) \
	    2>&1 | tee $(BUILD)/impl.log
	@echo "[impl] Done. See $(BUILD)/impl.log"

# ── Timing ────────────────────────────────────────────────────────────────
timing: $(ASC)
	@echo "[timing] Extracting from $(BUILD)/impl.log..."
	@grep -E "Max frequency|Worst slack|path\(s\)|Routing complete|Routing failed" \
	    $(BUILD)/impl.log 2>/dev/null \
	    || echo "(no timing output — check build/impl.log)"

# ── Bitstream pack ────────────────────────────────────────────────────────
pack: $(BIN)

$(BIN): $(ASC) | $(BUILD)
	@echo "[pack] icepack → $(BIN)"
	$(ICEPACK) $(ASC) $(BIN)
	@echo "[pack] Done: $(BIN)"

# ── Flash ─────────────────────────────────────────────────────────────────
flash: $(BIN)
	@echo "[flash] Programming $(BIN) to device..."
	$(ICEPROG) $(BIN)
	@echo "[flash] Done"

# ── Simulation ────────────────────────────────────────────────────────────
TARGET ?= tb_$(TOP)

sim:
	@echo "[sim] Building $(TARGET)..."
	@mkdir -p $(BUILD)/sim
	$(VERILATOR) --binary --timing \
	    -Wall -Wno-fatal -Wno-DECLFILENAME \
	    --top-module $(TARGET) \
	    --Mdir $(BUILD)/sim \
	    -o sim_$(TARGET) \
	    $(TBDIR)/$(TARGET).v $(SRCS)
	@echo "[sim] Running $(TARGET)..."
	$(BUILD)/sim/sim_$(TARGET)

# ── Clean ─────────────────────────────────────────────────────────────────
clean:
	rm -rf $(BUILD)
	@echo "[clean] Done"

# ── Info ──────────────────────────────────────────────────────────────────
info:
	@echo "=== Pclika HDL Platform ==="
	@echo "TOP:    $(TOP)"
	@echo "DEVICE: $(DEVICE) / $(PKG)"
	@echo "FREQ:   $(FREQ) MHz"
	@echo "SRCS:   $(SRCS)"
	@echo ""
	@echo "=== Tool versions ==="
	@$(YOSYS) --version 2>/dev/null     || echo "yosys: NOT FOUND"
	@$(NEXTPNR) --version 2>/dev/null   || echo "nextpnr-ice40: NOT FOUND"
	@$(VERILATOR) --version 2>/dev/null || echo "verilator: NOT FOUND"
	@$(ICEPROG) --version 2>/dev/null   || echo "iceprog: NOT FOUND"
