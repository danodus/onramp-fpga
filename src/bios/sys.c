// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "globals.h"
#include "common.h"
#include "io.h"

_Noreturn void sys_exit(int exit_code) {
    // Exit the simulation
    *(int *)(0x20000000) = exit_code;
    // Make sure we don't go further on the hardware
    for (;;);
}

int sys_time(unsigned out_buffer[3]) {
    unsigned long ms = *(unsigned int *)(0x30000004);
    ms <<= 32;
    ms |= *(unsigned int *)(0x30000000);
    unsigned long s = ms / 1000UL;
    unsigned long ns = 1000000 * (ms % 1000UL);

    out_buffer[0] = s & 0xFFFFFFFF;
    out_buffer[1] = s >> 32;
    out_buffer[2] = ns;
    return 0;
}

int sys_fopen(const char* path, bool writeable) {
    print("sys_fopen\n");
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    fs_context_t* fs_ctx = &bios_globals->fs_ctx;

    for (int i = 0; i < MAX_OPEN_FILES; ++i) {
        if (bios_globals->filenames[i][0] == '\0') {
            // empty slot found
            strncpy(bios_globals->filenames[i], path, FS_MAX_FILENAME_LEN);
            bios_globals->read_positions[i] = 0;
            bios_globals->write_positions[i] = 0;
            return 3 + i;
        }
    }

    return -1;
}

int sys_fclose(int file_handle) {
    print("sys_fclose\n");
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    bios_globals->filenames[file_handle - 3][0] = '\0';
    return 0;
}

int sys_fread(int handle, void* buffer, unsigned size) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;

    if (handle > 2) {
        print("sys_fread called\n");
        fs_context_t* fs_ctx = &bios_globals->fs_ctx;
        size_t nb_read_bytes;
        if (!fs_read(fs_ctx, bios_globals->filenames[handle - 3], buffer, bios_globals->read_positions[handle - 3], size, &nb_read_bytes)) {
            print("sys_fread: Unable to read\n");
            return 0;
        }
        bios_globals->read_positions[handle - 3] += nb_read_bytes;
        return nb_read_bytes;
    } else {
        // stdin
        if (size >= 1) {
            char c = getchar(0);    // non-blocking
            if (c) {
                *(char *)buffer = c;
                return 1;
            }
        }
    }
    return 0;
}

int sys_fwrite(int handle, const void* buffer, unsigned size) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;

    if (handle > 2) {
        print("sys_fwrite called\n");
        fs_context_t* fs_ctx = &bios_globals->fs_ctx;
        if (!fs_write(fs_ctx, bios_globals->filenames[handle - 3], buffer, bios_globals->write_positions[handle - 3], size)) {
            print("sys_fread: Unable to write\n");
            return 0;
        }
        bios_globals->write_positions[handle - 3] += size;
        return size;
    } else {
        // stdout/stderr
        for (unsigned i = 0; i < size; ++i) {
            putchar(*((char *)buffer));
            ++buffer;
        }
    }
    return size;
}

int sys_dopen(const char* path) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    fs_context_t* fs_ctx = &bios_globals->fs_ctx;

    fs_file_info_t file_info;
    bios_globals->dir_nb_files = fs_get_nb_files(fs_ctx);
    bios_globals->dir_file_index = 0;

    return 0;
}

int sys_dread(int handle, char out_buffer[256]) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    fs_context_t* fs_ctx = &bios_globals->fs_ctx;

    // If no more files are available
    if (bios_globals->dir_file_index >= bios_globals->dir_nb_files) {
        out_buffer[0] = '\0';
        return 0;
    }

    fs_file_info_t file_info;
    if (!fs_get_file_info(fs_ctx, bios_globals->dir_file_index, &file_info))
        return 1;

    for (size_t i = 0; i < sizeof(file_info.name); ++i)
        out_buffer[i] = file_info.name[i];

    bios_globals->dir_file_index++;

    return 0;
}

int sys_stat(const char* path, unsigned output[4]) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    fs_context_t* fs_ctx = &bios_globals->fs_ctx;
    
    uint16_t nb_files = fs_get_nb_files(fs_ctx);
    bool found = false;
    for (uint16_t i = 0; i < nb_files; ++i) {
        fs_file_info_t file_info;
        if (!fs_get_file_info(fs_ctx, i, &file_info))
            break;
        if (strcmp(file_info.name, path) == 0) {
            output[0] = 0;              // type = file
            output[1] = 493;            // mode = executable
            output[2] = file_info.size; // size_low
            output[3] = 0;              // size_high
            found = true;
        }
    }
    return found ? 0 : 1;
}

