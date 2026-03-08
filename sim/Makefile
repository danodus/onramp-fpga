VERILATOR = verilator

LDFLAGS := 
CFLAGS := -CFLAGS "--std=c++17"

DEFINES = -DSDRAM

all: sim

clean:
	rm -rf obj_dir
	rm -f ../sd.img

sim: top.v sim_main.cpp
	$(VERILATOR) -cc --exe $(CFLAGS) $(LDFLAGS) top.v sim_main.cpp $(DEFINES) -I../rtl --trace -Wno-NULLPORT -Wno-PINMISSING
	$(MAKE) -j 4 -C obj_dir -f Vtop.mk

run: sim
	cd ../src/bios && ./build.sh
	cd ../scripts && ./mkimg.sh
	obj_dir/Vtop || true

.PHONY: all clean
