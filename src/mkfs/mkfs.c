// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include <fs.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: mkfs img_path\n");
        return 1;
    }

    // Initialize the SD card
    if (!sdc_init(argv[1])) {
        printf("Unable to initialize the SD card\r\n");
        return 1;
    }
    printf("SD card image initialized\r\n");

    // Format the SD card
    printf("Formatting the SD card image...\r\n");
    if (!fs_format(false)) {
        printf("Unable to format the SD card image \r\n");
        sdc_dispose();
        return 1;
    }
    printf("SD card image formatted\r\n");

    fs_context_t fs_ctx;
    if (!fs_init(&fs_ctx)) {
        printf("Unable to mount the FS\r\n");
        sdc_dispose();
        return 1;
    }

    if (!fs_write(&fs_ctx, "README.txt", (const uint8_t*)"Content of README", 0, 18)) {
        printf("Unable to write to a file\r\n");
        sdc_dispose();
        return 1;
    }

    sdc_dispose();

    return 0;
}