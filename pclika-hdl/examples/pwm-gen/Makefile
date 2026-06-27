# Makefile — pwm-gen example
# Seal: PCK-MMXXVI-9198580D

TOP    ?= pwm_gen_top
DEVICE ?= up5k
PKG    ?= sg48
FREQ   ?= 12
SEED   ?= 1

IP_RTL := ../../hdl/rtl

YOSYS    ?= yosys
NEXTPNR  ?= nextpnr-ice40
ICEPACK  ?= icepack
ICEPROG  ?= iceprog
VERILATOR ?= verilator

SYNTH_JSON := build/$(TOP).json
ROUTED     := build/$(TOP).asc
BITSTREAM  := build/$(TOP).bin

RTL_SRCS := rtl/$(TOP).v \
            $(IP_RTL)/pclika_pwm.v

.PHONY: all lint synth impl pack flash clean

all: pack

build:
	mkdir -p build

lint:
	$(VERILATOR) --lint-only -Wall --top-module $(TOP) $(RTL_SRCS)

synth: build lint
	$(YOSYS) -p "read_verilog $(RTL_SRCS); \
	             synth_ice40 -top $(TOP) -json $(SYNTH_JSON)" \
	         -l build/synth.log

impl: synth
	$(NEXTPNR) --$(DEVICE) --package $(PKG) --freq $(FREQ) \
	           --seed $(SEED) \
	           --json $(SYNTH_JSON) \
	           --pcf constraints/ice40up5k.pcf \
	           --asc $(ROUTED) \
	           --log build/impl.log

pack: impl
	$(ICEPACK) $(ROUTED) $(BITSTREAM)
	@echo "Bitstream ready: $(BITSTREAM)"

flash: pack
	$(ICEPROG) $(BITSTREAM)

clean:
	rm -rf build
