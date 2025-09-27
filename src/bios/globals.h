// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef GLOBALS_H
#define GLOBALS_H

#include <config.h>
#include <fs.h>
#include <conio.h>

#define BIOS_GLOBALS 0x11f80000  // (32 MiB - 512 KiB)


typedef struct {
    uint16_t file_index;
    size_t position;    // read or write position for ftell/fseek
    unsigned long io_time;
} file_t;

typedef struct {
    fs_context_t fs_ctx;
    conio_context_t conio_ctx;

    // Files
    file_t files[MAX_OPEN_FILES];

    // Directory listing
    uint16_t dir_nb_files;
    uint16_t dir_file_index;
} bios_globals_t;

#endif
