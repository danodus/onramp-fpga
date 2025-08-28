// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <spawn.h>

#include <fs.h>

extern unsigned* __process_info_table;

void set_led(int value) {
    *(int *)(0x2000F000) = value;
}

void exit_handler(void) {
    set_led(0x55);
}

uint32_t get_file_size(fs_context_t* fs_ctx, const char* filename) {
    fs_file_info_t file_info;
    uint16_t nb_files = fs_get_nb_files(fs_ctx);
    for (uint16_t i = 0; i < nb_files; ++i) {
        if (fs_get_file_info(fs_ctx, i, &file_info)) {
            if (strcmp(filename, file_info.name) == 0)
                return file_info.size;
        }
    }
    return 0;
}

bool run_program(fs_context_t* fs_ctx, const char* filename) {
    uint32_t program_size = get_file_size(fs_ctx, filename);
    
    if (program_size == 0) {
        printf("%s is not found or empty\n", filename);
        return false;
    }

    //
    // Execute hex.oe
    //

    char *program = malloc(program_size);
    if (program == NULL) {
        printf("Out of memory\n");
        return false;
    }

    if (!fs_read(fs_ctx, filename, (uint8_t*)program, 0, program_size, NULL)) {
        printf("Unable to read the program\n");
        free(program);
        return false;
    }

    // Check if this is an Onramp program
    if (strncmp(program, "~Onr~amp~   ", 12) != 0) {
        printf("Not an Onramp program\n");
        free(program);
        return false;
    }

    printf("Execute %s...\n", filename);

    // Allocate a process information table for the child as a copy of ours
    unsigned int* parent_pit = __process_info_table;
    unsigned int* child_pit = __memdup(parent_pit, sizeof(int) * 12);

    // Setup the child pit
    //*(child_pit + PIT_ARGS) = (int)argv;
    //*(child_pit + PIT_ENVIRON) = (int)environ;

    // Run it
    int ret = __onramp_spawn_pit(program, program_size, child_pit, filename);
    
    free(program);
    return true;
}

int main(void) {

    atexit(exit_handler);
    printf("Onramp-FPGA OS\n");

    // Initialize the SD card
    if (!sdc_init()) {
        printf("Unable to initialize the SD card\n");
        return 1;
    }

    fs_context_t fs_ctx;
    fs_init(&fs_ctx);

    if (!run_program(&fs_ctx, "hex.oe")) {
        printf("Unable to run the program\n");
        sdc_dispose();
        return 1;
    }

    printf("Success!\n");

    sdc_dispose();

    return 0;
}