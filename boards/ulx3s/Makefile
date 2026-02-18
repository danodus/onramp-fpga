SERIAL ?= /dev/ttyUSB0

SRC =  top.v \
	pll.v \
	pll_video.v \
	../../rtl/div.v \
	../../rtl/or32.v \
	../../rtl/sdram.v \
	../../rtl/cache_controller.v \
	../../rtl/bram32bit.v \
	../../rtl/bram_true2p_2clk.v \
	../../rtl/timer.v \
	../../rtl/soc.v \
	../common/cfg.v \
	../common/led.v \
	../common/uart.v \
	../common/uart_rx.v \
	../common/uart_tx.v \
	../common/spi.v \
	../common/ps2.v \
	../common/ps2_kbd.v \
	../common/video/font_cp437_8x8.v \
	../common/video/tile_renderer.v \
	../common/video/cursor_renderer.v \
	../common/video/vram.v \
	../common/video/hvsync_generator.v \
	../common/video/vdu.v \
	../common/hdmi_encoder.v \
	../common/soc_ext.v

#TOOLPREFIX = /opt/oss-cad-suite/bin/
TOOLPREFIX =

PNRFLAGS = --85k --package CABGA381 --speed 6 --lpf ulx3s.lpf --randomize-seed --timing-allow-fail

DEFINES = -DSDRAM

all:	build

build:	soc.bit

soc.json: $(SRC)
	cd ../../src/bios && ./build.sh
	$(TOOLPREFIX)yosys -ql top.log -p 'verilog_defines $(DEFINES) ; read_verilog -sv $(SRC); synth_ecp5 -top top -json $@'

soc_out.config: soc.json
	$(TOOLPREFIX)nextpnr-ecp5 $(PNRFLAGS) --json $< --textcfg $@

soc.bit: soc_out.config
	$(TOOLPREFIX)ecppack --svf soc.svf $< $@

clean:
	rm -f *.asc *.bin *.json *.vcd *.out *.svf *.config *.bit

prog: soc.bit
	$(TOOLPREFIX)fujprog soc.bit

run:
	cd ../../src/os && ./build.sh
	python3 ../../scripts/sendhex.py $(SERIAL) ../../src/os/shell.hex

.PHONY: all clean prog run
