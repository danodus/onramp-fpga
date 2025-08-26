SRC =  top.v ../rtl/div.v ../rtl/or32.v ../rtl/timer.v ../rtl/soc.v led.v

#TOOLPREFIX = /opt/oss-cad-suite/bin/
TOOLPREFIX =

PNRFLAGS = --85k --package CABGA381 --speed 6 --lpf ulx3s.lpf

DEFINES =

all:	build

sim:	soc_tb.vcd

build:	soc.bit

soc.json: $(SRC)
	cd ../src/bios && ./build.sh
	cd ../src/os && ./build.sh
	$(TOOLPREFIX)yosys -ql top.log -p 'verilog_defines $(DEFINES) ; read_verilog -sv $(SRC); synth_ecp5 -top top -json $@'

soc_out.config: soc.json
	$(TOOLPREFIX)nextpnr-ecp5 $(PNRFLAGS) --json $< --textcfg $@

soc.bit: soc_out.config
	$(TOOLPREFIX)ecppack --svf soc.svf $< $@

clean:
	rm -f *.asc *.bin *.json *.vcd *.out *.svf *.config *.bit

prog: soc.bit
	$(TOOLPREFIX)fujprog soc.bit

.PHONY: all clean prog
