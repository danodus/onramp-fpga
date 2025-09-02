# Memory Map

The memory map is the following:

| Region | Description |
| ------ | ----------- |
| 0x00000000-0x0FFFFFFF | ROM with the BIOS (32 KiB) |
| 0x10000000-0x1FFFFFFF | RAM with the operating system (256 KiB for now. I will eventually add SDRAM support for up to 32 MiB on my board) |
| 0x20000000-0x2000FFFF | External BUS |
| 0x30000000-0x3000000F | Timer |

## External Bus

### Simulator

With the simulator, the external bus has the following memory-mapped devices:

| Address | Description |
| ------- | ----------- |
| 0x20000000 | Exit with exit code |
| 0x20000004 | Write a character to console |
| 0x20000008 | Read a character from console (0 if none) |
| 0x2000000C | Set simulated SD card address |
| 0x20000010 | Read/write byte from simulated SD card |
| 0x2000F000 | Set LEDs |

### ULX3S

Will always acknowledge but only recognize the LED device.

## Timer

The timer has the following memory-mapped registers:

| Address | Description |
| ------- | ----------- |
| 0x30000000 | Seconds low 32-bit |
| 0x30000004 | Seconds high 32-bit |
| 0x30000008 | Nanoseconds |

# Onramp Toolchain

I'm only supporting the POSIX build of Onramp.  You can build the toolchain with these commands:

```bash
git clone https://github.com/ludocode/onramp
cd onramp
./scripts/posix/build.sh
export ONRAMP_BIN=`pwd`/build/posix/bin
```

With the `onrampcc` script in the `build/posix/bin` folder, a Posix wrapper is added around the final binary with the `-wrap-header`. To avoid this option, I execute `cc.oe` with the VM directly.

# BIOS

I build the BIOS as freestanding with `-nostdlib` and `-nostdinc`.  I therefore provide myself the start code with the following steps:
1) Set the stack pointer to the end of the RAM.
2) Set r0 to the address of my process info table filled with my low-level IO routines.
3) Set the rpp register to the start of RAM.  This register is used as a base offset for calls.
4) Jumps to the start of RAM.  This is the kernel entry point.

# Kernel

# Structure Initialization

With Onramp C, unlike GCC, structure initialization for a global variable requires a constructor.  For this reason, the BIOS being in a read-only memory region cannot initialize a structure like this:

```C
typedef struct {
    int v;
} mystruct_t;

mystruct_t v = {
    3
};
```

# System Calls

The BIOS offers a subset of system calls required for Onramp libc used by the kernel. Includes:

Num | Description
--- | -----------
0 | exit with status
2 | time (see below)
5 | fread
6 | fwrite

The kernel includes a file system. It will therefore provide a more complete list of system calls to the programs being executed from the shell.

With libc, `__start_c` calls `__time_setup`.  This function is calling `__sys_time` therefore this system call *must* be implemented.

# Known Issues

# Next Steps

Run hex-0 with argument.  We need to connect the file system calls to the child PIT.
