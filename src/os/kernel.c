// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <fs.h>

void set_led(int value) {
    *(int *)(0x2000F000) = value;
}

void exit_handler(void) {
    set_led(0x55);
}

void ls(fs_context_t* fs_ctx) {
    fs_file_info_t file_info;
    uint16_t nb_files = fs_get_nb_files(fs_ctx);
    for (uint16_t i = 0; i < nb_files; ++i) {
        if (fs_get_file_info(fs_ctx, i, &file_info)) {
            printf("%s\t%d\r\n", file_info.name, file_info.size);
            char *buf = malloc(file_info.size);
            if (!fs_read(fs_ctx, file_info.name, (uint8_t *)buf, 0, file_info.size, NULL)) {
                printf("Unable to read\r\n");
                return;
            }
            printf("  Content: %s\r\n", buf);
            free(buf);
        }
    }
}

int main(void) {

    atexit(exit_handler);
    printf("Onramp-FPGA OS\r\n");

    // Initialize the SD card
    if (!sdc_init()) {
        printf("Unable to initialize the SD card\r\n");
        return 1;
    }

    fs_context_t fs_ctx;
    fs_init(&fs_ctx);

    if (!fs_write(&fs_ctx, "test.txt", (const uint8_t*)"Hello, World!", 0, 14)) {
        printf("Unable to write to a file\r\n");
        sdc_dispose();
        return 1;
    }

    ls(&fs_ctx);
    sdc_dispose();

    return 0;
}