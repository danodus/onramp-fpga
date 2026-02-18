// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "globals.h"
#include "io.h"
#include "common.h"

#include <conio.h>

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

int main(void) {

    // TODO: Is there a way to use static assert instead?
    if (BIOS_GLOBALS + sizeof(bios_globals_t) > 0x12000000) {
        // BIOS globals too large. System halted.
        for(;;);
    }

    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    if (is_hardware()) {
        conio_init(&bios_globals->conio_ctx);
        print("Running on hardware\n");
    } else {
        print("Running on the simulator\n");
    }

    for (int i = 0; i < MAX_OPEN_FILES; ++i) {
        file_t* f = &bios_globals->files[i];
        f->file_index = FS_INVALID_INDEX;
    }

    print("BIOS: Initialized\n");

    // Initialize the SD card
    if (!sdc_init()) {
        print("Unable to initialize the SD card. System halted.\n");
        for(;;);
    }

    if (!fs_mount(&bios_globals->fs_ctx)) {
        print("Invalid FS image. System halted.\n");
        for(;;);
    }

    // Load the shell from SD card
    bool is_shell_loaded = false;
    uint16_t file_index = fs_find_file(&bios_globals->fs_ctx, "shell.oe");
    if (file_index != FS_INVALID_INDEX) {
        bool is_load_bypassed = false;
        if (is_hardware()) {
            print("Press a key to bypass the SD card boot process...\n");
            unsigned long target_clock = clock() + 4000;
            for (;;) {
                if (getchar(0)) {
                    is_load_bypassed = true;
                    break;
                } else if (clock() > target_clock) {
                    break;
                }
            }
        }
        if (!is_load_bypassed) {
            print("Loading the shell from SD card...\n");
            fs_file_info_t file_info;
            fs_get_file_info(&bios_globals->fs_ctx, file_index, &file_info, true);
            is_shell_loaded = fs_read(&bios_globals->fs_ctx, file_index, (uint8_t *)RAM_START, 0, file_info.size, NULL);
        }
    }

    if (!is_shell_loaded)
        receive();

    if (*(unsigned int *)RAM_START != 0x726e4f7e ||
        *(unsigned int *)(RAM_START + 4) != 0x706d617e ||
        *(unsigned int *)(RAM_START + 8) != 0x2020207e) {
            print("Unknown program. System Halted.\n");
            for (;;);
        }

    print("Starting...\n");
    return 0;
}