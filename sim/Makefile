VERILATOR = verilator

LDFLAGS := -LDFLAGS "$(shell sdl2-config --libs)"
CFLAGS := -CFLAGS "--std=c++14 $(shell sdl2-config --cflags)"

DEFINES = -DUART -DSDRAM -DSDCARD -DPS2 -DVIDEO

all: sim

clean:
	rm -rf obj_dir

sim: top.v sim_main.cpp
	$(VERILATOR) -cc --exe $(CFLAGS) $(LDFLAGS) top.v sdl_ps2.cpp sim_main.cpp $(DEFINES) -I../rtl --trace -Wno-NULLPORT -Wno-PINMISSING
	$(MAKE) -j 4 -C obj_dir -f Vtop.mk

run: sim
	obj_dir/Vtop

.PHONY: all clean
