// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef GLOBALS_H
#define GLOBALS_H

#include <config.h>
#include <fs.h>

#define BIOS_GLOBALS 0x11fc0000  // (32 MiB - 256 KiB)

#define IO_BUFFER_SIZE  4096

typedef struct {
    uint8_t data[IO_BUFFER_SIZE];
    size_t count;
} io_buffer_t;

typedef struct {
    uint16_t file_index;
    size_t read_position;
    size_t write_position;
    io_buffer_t read_buf, write_buf;
    size_t read_buf_offset;
    size_t position;    // read or write position for ftell/fseek
} file_t;

typedef struct {
    fs_context_t fs_ctx;

    // Files
    file_t files[MAX_OPEN_FILES];

    // Directory listing
    uint16_t dir_nb_files;
    uint16_t dir_file_index;
} bios_globals_t;

#endif
