// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "globals.h"
#include "common.h"
#include "io.h"

_Noreturn void sys_exit(int exit_code) {
    // Exit the simulation
    *(int *)(0x20000000) = exit_code;
    // Make sure we don't go further on the hardware
    for (;;);
}

int sys_time(unsigned out_buffer[3]) {
    unsigned long ms = *(unsigned int *)(0x30000004);
    ms <<= 32;
    ms |= *(unsigned int *)(0x30000000);
    unsigned long s = ms / 1000UL;
    unsigned long ns = 1000000 * (ms % 1000UL);

    out_buffer[0] = s & 0xFFFFFFFF;
    out_buffer[1] = s >> 32;
    out_buffer[2] = ns;
    return 0;
}

int sys_fopen(const char* path, bool writeable) {
    print("sys_fopen\n");
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;

    return -1;
}

int sys_fclose(int file_handle) {
    print("sys_fclose\n");
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    return 0;
}

int sys_fread(int handle, void* buffer, unsigned size) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;

    if (handle > 2) {
        print("sys_fread called\n");
        return -1;
    } else {
        // stdin
        if (size >= 1) {
            char c = getchar(0);    // non-blocking
            if (c) {
                *(char *)buffer = c;
                return 1;
            }
        }
    }
    return 0;
}

int sys_fwrite(int handle, const void* buffer, unsigned size) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;

    if (handle > 2) {
        print("sys_fwrite called\n");
        return 0;
    } else {
        // stdout/stderr
        for (unsigned i = 0; i < size; ++i) {
            putchar(*((char *)buffer));
            ++buffer;
        }
    }
    return size;
}

int sys_dopen(const char* path) {
    bios_globals_t* g = (bios_globals_t*)BIOS_GLOBALS;

    struct inode *ip;
    ip = namei(path);
    if (ip == 0)
        return -1;

    return 0;
}

int sys_dread(int handle, char out_buffer[256]) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    return -1;
}

int sys_stat(const char* path, unsigned output[4]) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    return -1;
}

