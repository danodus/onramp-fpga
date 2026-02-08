# Onramp-FPGA

Onramp-FPGA is a FPGA-based system-on-chip with a 32-bit Onramp processor.

The goal of this project is to build and run the Onramp toolchain directly on a FPGA board.

Refer to my [notes](NOTES.md) for more details.

## Requirements

- Onramp installation (https://github.com/ludocode/onramp)
- OSS CAD Suite (https://github.com/YosysHQ/oss-cad-suite-build)

## Getting Started

### Simulator

```bash
source /path/to/oss-cad-suite/environment
cd sim
make run ONRAMP_BIN=/path/to/onramp/output/posix/bin
```

### ULX3S FPGA Board

For the keyboard, you need a Digilent PS/2 PMOD connected to the top-left corner of the board.

Generate a SD card image:

```bash
export ONRAMP_BIN=/path/to/onramp/output/posix/bin
cd scripts
./mkimg.sh
cd ..
```

Flash the SD card image `sd.img`.

```bash
source /path/to/oss-cad-suite/environment
cd ulx3s
make prog
```

## Build Steps

### Hello World

To edit, do the following at the prompt:

```sh
ed main.c
```

To build and run, do the following:

```sh
sh build.sh
main
```

### Toolchain

The pre-built toolchain is already present on the image.  It is however possible to build it from scratch on the board directly with the following steps:

```sh
rmall output/
rm sh.oe
hex core/sh/sh.oe.ohx -o sh.oe
cp hex.oe output/intermediate/hex-0-onramp/hex.oe
cp sh.oe output/intermediate/sh/sh.oe
sh core/build.sh
```

Note: The build process currently takes about 6 hours on the ULX3S.
