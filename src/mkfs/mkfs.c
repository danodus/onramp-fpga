// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fs.h>

void print(const char *s) {
    printf("%s", s);
}

bool add_file(fs_context_t* fs_ctx, const char* filepath) {
    // Get the filename
    char* path = strdup(filepath);
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
        free(path);
        return false;
    }

    printf("Adding file %s...\n", dstpath);

    uint16_t file_index = fs_create_file(fs_ctx, dstpath);
    if (file_index == FS_INVALID_INDEX) {
        printf("Unable to create %s\n", dstpath);
        fclose(f);
        free(path);
        return false;
    }

    uint8_t buf[512];
    size_t total_size = 0;
    for (;;) {
        size_t n = fread(buf, 1, sizeof(buf), f);
        if (n <= 0)
            break;
        if (!fs_write(fs_ctx, file_index, buf, total_size, n)) {
            printf("Unable to write %s\n", dstpath);
            fclose(f);
            free(path);
            return false;
        }
        total_size += n;
    }

    fs_sync(fs_ctx);

    fclose(f);
    free(path);
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

    fs_context_t* fs_ctx = malloc(sizeof(fs_context_t));

    // Format the SD card
    printf("Formatting the SD card image...\r\n");
    if (!fs_format(fs_ctx, false)) {
        printf("Unable to format the SD card image \r\n");
        free(fs_ctx);
        sdc_img_dispose();
        return 1;
    }
    printf("SD card image formatted\r\n");

    if (!fs_mount(fs_ctx)) {
        printf("Unable to mount the FS\r\n");
        free(fs_ctx);
        sdc_img_dispose();
        return 1;
    }

    bool success = true;
    for (int arg = 2; arg < argc; arg++) {
        if (!add_file(fs_ctx, argv[arg])) {
            success = false;
            break;
        }
    }

    free(fs_ctx);

    sdc_img_dispose();

    return success ? 0 : 1;
}