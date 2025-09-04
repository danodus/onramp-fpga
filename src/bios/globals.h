// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef GLOBALS_H
#define GLOBALS_H

#include <fs.h>

#define BIOS_GLOBALS 0x1003f000

#define MAX_OPEN_FILES  4

typedef struct {
    fs_context_t fs_ctx;

    // Files
    char filenames[MAX_OPEN_FILES][FS_MAX_FILENAME_LEN + 1];
    size_t read_positions[MAX_OPEN_FILES];
    size_t write_positions[MAX_OPEN_FILES];

    // Directory listing
    uint16_t dir_nb_files;
    uint16_t dir_file_index;
} bios_globals_t;

#endif
