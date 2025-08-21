# Onramp-FPGA

Onramp-FPGA is a FPGA-based system-on-chip with a 32-bit Onramp processor.

The goal of this project is to have the Onramp toolchain running directly on a FPGA board.

## Requirements

- Onramp installation (https://github.com/ludocode/onramp)
- OSS CAD Suite (https://github.com/YosysHQ/oss-cad-suite-build)

## Getting started

### Build the BIOS using the Onramp toolchain

```bash
cd src/bios
export ONRAMP_BIN=/path/to/onramp/build/posix/bin
source build.sh
cd ../..
```

### Execution on the simulator

```bash
source /path/to/oss_cad_suite/environment
cd sim
make run
```

### Execution on the ULX3S FPGA Board

```bash
source /path/to/oss-cad-suite/environment
cd ulx3s
make prog
```
