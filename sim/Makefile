VERILATOR = verilator

LDFLAGS := 
CFLAGS := -CFLAGS "--std=c++14"

DEFINES = -DSDRAM

all: sim

clean:
	rm -rf obj_dir
	rm -f sd.img

sim: top.v sim_main.cpp
	$(VERILATOR) -cc --exe $(CFLAGS) $(LDFLAGS) top.v sim_main.cpp $(DEFINES) -I../rtl sdram.v bram32bit.v bram_true2p_2clk.v --trace -Wno-NULLPORT -Wno-PINMISSING -Wno-WIDTHEXPAND -Wno-CASEINCOMPLETE -Wno-WIDTHTRUNC -Wno-MULTIDRIVEN
	$(MAKE) -j 4 -C obj_dir -f Vtop.mk

run: sim
	cd ../src/bios && ./build.sh
	cd ../scripts && ./mkimg.sh
	obj_dir/Vtop || true

.PHONY: all clean
