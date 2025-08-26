# Onramp-FPGA

Onramp-FPGA is a FPGA-based system-on-chip with a 32-bit Onramp processor.

The goal of this project is to have the Onramp toolchain running directly on a FPGA board.

Refer to my [notes](NOTES.md) for more details.

## Requirements

- Onramp installation (https://github.com/ludocode/onramp)
- OSS CAD Suite (https://github.com/YosysHQ/oss-cad-suite-build)

## Getting started

### Execution on the simulator

```bash
source /path/to/oss_cad_suite/environment
cd sim
make run ONRAMP_BIN=/path/to/onramp/build/posix/bin
```

### Execution on the ULX3S FPGA Board

```bash
source /path/to/oss_cad_suite/environment
cd ulx3s
make prog ONRAMP_BIN=/path/to/onramp/build/posix/bin
```
