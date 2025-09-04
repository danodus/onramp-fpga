// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "globals.h"
#include "io.h"

#define RAM_START 0x10000000

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

    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    for (int i = 0; i < MAX_OPEN_FILES; ++i)
        bios_globals->filenames[i][0] = '\0';

    print("BIOS: Initialized\n");

    // Initialize the SD card
    if (!sdc_init()) {
        print("Unable to initialize the SD card\n");
        return 1;
    }

    if (!fs_init(&bios_globals->fs_ctx)) {
        print("Invalid FS image\n");
        return 1;
    }

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

    print("Starting...\n");
    return 0;
}