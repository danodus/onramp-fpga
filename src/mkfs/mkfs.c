// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <string.h>

#include <fs.h>

void print(const char *s) {
    printf("%s", s);
}

bool add_file(fs_context_t* fs_ctx, const char* filepath) {
    // Get the filename
    char path[256];
    strncpy(path, filepath, sizeof(path));
    char* srcpath = path;
    char* dstpath = strrchr(path, ':');
    if (dstpath) {
         // skip delimiter
        *dstpath = '\0';
        ++dstpath;
    } else {
        dstpath = srcpath;
    }

    FILE *f = fopen(srcpath, "rb");
    if (f == NULL) {
        printf("Unable to open %s\n", srcpath);
        return false;
    }

    printf("Adding file %s...\n", dstpath);

    uint8_t buf[512];
    size_t total_size = 0;
    for (;;) {
        size_t n = fread(buf, 1, sizeof(buf), f);
        if (n <= 0)
            break;
        if (!fs_write(fs_ctx, dstpath, buf, total_size, n)) {
            printf("Unable to write %s\n", dstpath);
            return false;
        }

        total_size += n;
    }   

    fclose(f);
    return true;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: mkfs img [file ...]\n");
        return 1;
    }

    // Initialize the SD card
    if (!sdc_img_init(argv[1])) {
        printf("Unable to initialize the SD card\r\n");
        return 1;
    }
    printf("SD card image initialized\r\n");

    // Format the SD card
    printf("Formatting the SD card image...\r\n");
    if (!fs_format(false)) {
        printf("Unable to format the SD card image \r\n");
        sdc_img_dispose();
        return 1;
    }
    printf("SD card image formatted\r\n");

    fs_context_t fs_ctx;
    if (!fs_init(&fs_ctx)) {
        printf("Unable to mount the FS\r\n");
        sdc_img_dispose();
        return 1;
    }

    bool success = true;
    for (int arg = 2; arg < argc; arg++) {
        if (!add_file(&fs_ctx, argv[arg])) {
            success = false;
            break;
        }
    }

    sdc_img_dispose();

    return success ? 0 : 1;
}