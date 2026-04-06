# Onramp-FPGA

Onramp-FPGA is a FPGA-based system-on-chip with a 32-bit Onramp processor.

The goal of this project is to build and run the Onramp toolchain directly on a FPGA board.

Refer to my [notes](NOTES.md) for more details.

## Requirements

- Onramp installation (https://github.com/ludocode/onramp)
- OSS CAD Suite (https://github.com/YosysHQ/oss-cad-suite-build)

Tested with Onramp 7ce9870db8c63aa13ca8dffafd0350245678bde3.

## Getting Started

### Simulator

```bash
source /path/to/oss-cad-suite/environment
cd sim
make run ONRAMP_BIN=/path/to/onramp/output/posix/bin
```

### FPGA Board

The FPGA implementation provides two I/O interfaces: serial and video output + keyboard.  Both interfaces can be used simultaneously.

Generate a SD card image:

```bash
export ONRAMP_BIN=/path/to/onramp/output/posix/bin
cd scripts
./mkimg.sh
cd ..
```

Flash the SD card image `sd.img` and insert it into the FPGA board.

#### ULX3S

```bash
source /path/to/oss-cad-suite/environment
cd boards/ulx3s
make prog
picocom -b 115200 --imap lfcrlf <serial device>
```

For the keyboard, you need a Digilent PS/2 PMOD connected to the top-left corner of the board.

#### Icepi Zero

```bash
source /path/to/oss-cad-suite/environment
cd boards/icepi-zero
make prog
picocom -b 115200 --imap lfcrlf <serial device>
```

The PS/2 keyboard must be connected to USB1.

## Build Steps

### Hello World

To edit, do the following at the prompt:

```sh
ed main.c
```

Note: If the serial port is used, the terminal must provide at least 80x60 characters.

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
cp hex.oe output/configure/hex-0-onramp/hex.oe
cp sh.oe output/configure/sh/sh.oe
time sh core/build.sh
```

Note: The build process currently takes 14698 seconds (about 4 hours) on the FPGA board.
