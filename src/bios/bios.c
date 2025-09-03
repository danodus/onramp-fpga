// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "io.h"

#define RAM_START 0x10000000

typedef unsigned long size_t;

static void print(char *s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}

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

int sys_fread(int handle, void* buffer, unsigned size) {
    if (size >= 1) {
        char c = getchar(0);    // non-blocking
        if (c) {
            *(char *)buffer = c;
            return 1;
        }
    }
    return 0;
}

int sys_fwrite(int handle, const void* buffer, unsigned size) {
    for (unsigned i = 0; i < size; ++i) {
        putchar(*((char *)buffer));
        ++buffer;
    }
    return size;
}

static void receive(void) {
    set_led(0xFF);
    print("Ready to receive...\n");

    // Read program
    unsigned int addr = RAM_START;
    unsigned int size;
    size = receive_word();

    if (size == 0) {
        set_led(0x01);
        return;
    }

    for (unsigned int i = 0; i < size; ++i) {
        unsigned int word = receive_word();
        *(volatile unsigned int *)addr = word;
        addr += 4;
        set_led(i << 1);        
    }
    set_led(0x00);
    print("Program received.\n");
}

// ref: https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
static char* uitoa(unsigned int value, char* result, int base)
{
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    for (int i = 0; i < 8; ++i) {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    }

    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

int main(void) {
    print("BIOS: Initialized\n");
    if (is_hardware()) {
        print("Running on hardware\n");
        receive();
    } else {
        print("Running on the simulator\n");
    }

    if (*(unsigned int *)RAM_START != 0x726e4f7e ||
        *(unsigned int *)(RAM_START + 4) != 0x706d617e ||
        *(unsigned int *)(RAM_START + 8) != 0x2020207e) {
            print("Unknown program. System Halted.\n");
            for (;;);
        }

/*
    char buf[32];
    for (int i = 0; i < 512; i += 4) {
        uitoa(*(unsigned int *)(RAM_START + i), buf, 16);
        print(buf);
        print(" ");
    }
*/

    print("Starting...\n");
    return 0;
}