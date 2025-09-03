# Onramp-FPGA

Onramp-FPGA is a FPGA-based system-on-chip with a 32-bit Onramp processor.

The goal of this project is to have the Onramp toolchain running directly on a FPGA board.

Refer to my [notes](NOTES.md) for more details.

## Requirements

- Onramp installation (https://github.com/ludocode/onramp)
- OSS CAD Suite (https://github.com/YosysHQ/oss-cad-suite-build)

## Getting Started

### Simulator

```bash
source /path/to/oss-cad-suite/environment
cd sim
make run ONRAMP_BIN=/path/to/onramp/build/posix/bin
```

### ULX3S FPGA Board

Generate a SD card image:

```bash
cd scripts
./mkimg.sh
cd ..
```

Flash the SD card image `sd.img`.

```bash
source /path/to/oss-cad-suite/environment
cd ulx3s
make prog run ONRAMP_BIN=/path/to/onramp/build/posix/bin SERIAL=/dev/ttyUSB0
```

Open a terminal at 115200-N-8-1 and press the enter key.
