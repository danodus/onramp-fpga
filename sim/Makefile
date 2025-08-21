VERILATOR = verilator

LDFLAGS := 
CFLAGS := -CFLAGS "--std=c++14"

DEFINES =

all: sim

clean:
	rm -rf obj_dir

sim: top.v sim_main.cpp
	$(VERILATOR) -cc --exe $(CFLAGS) $(LDFLAGS) top.v sim_main.cpp $(DEFINES) -I../rtl --trace -Wno-NULLPORT -Wno-PINMISSING
	$(MAKE) -j 4 -C obj_dir -f Vtop.mk

run: sim
	obj_dir/Vtop

.PHONY: all clean
