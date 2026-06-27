# Makefile — uart-echo example
# Seal: PCK-MMXXVI-9198580D

TOP    ?= uart_echo_top
DEVICE ?= up5k
PKG    ?= sg48
FREQ   ?= 12
SEED   ?= 1

# IP library RTL path
IP_RTL := ../../hdl/rtl

# Toolchain
YOSYS    ?= yosys
NEXTPNR  ?= nextpnr-ice40
ICEPACK  ?= icepack
ICEPROG  ?= iceprog
VERILATOR ?= verilator

SYNTH_JSON := build/$(TOP).json
ROUTED     := build/$(TOP).asc
BITSTREAM  := build/$(TOP).bin

RTL_SRCS := rtl/$(TOP).v \
            $(IP_RTL)/pclika_uart_rx.v \
            $(IP_RTL)/pclika_uart_tx.v

TB_SRCS  := tb/tb_$(TOP).v $(RTL_SRCS)

.PHONY: all lint synth impl pack flash sim clean

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

sim:
	mkdir -p build/sim
	$(VERILATOR) --cc --exe --build --timing \
	             -Irtl -I$(IP_RTL) \
	             --top-module tb_uart_echo_top \
	             $(TB_SRCS) \
	             -o build/sim/tb_uart_echo_top
	./build/sim/tb_uart_echo_top

clean:
	rm -rf build
