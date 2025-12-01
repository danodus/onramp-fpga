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
    unsigned int ms = *(unsigned int *)(0x30000000);
    unsigned int s = ms / 1000;
    unsigned int ns = 1000000 * (ms % 1000);

    out_buffer[0] = s;
    out_buffer[1] = 0;
    out_buffer[2] = ns;
    return 0;
}

int sys_fopen(const char* path, bool writeable) {
    //print("sys_fopen\n");

    // Ignore the / and ./ prefixes from the current toolchain

    if (path[0] == '/')
        path++;

    if (path[0] == '.' && path[1] == '/')
        path += 2;

    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    fs_context_t* fs_ctx = &bios_globals->fs_ctx;

    // Make sure the file exists if not writeable
    if (!writeable) {
        //print("sys_fopen: \"");
        //print(path);
        if (!fs_file_exists(fs_ctx, path)) {
            //print("\" not found\n");
            return -1;
        }
        // else {
        //    print("\" found\n");
        //}
    }

    for (int i = 0; i < MAX_OPEN_FILES; ++i) {
        file_t* f = &bios_globals->files[i];
        if (f->file_index == FS_INVALID_INDEX) {
            // empty slot found

            // If the file is writeable and does not exist, create an empty file
            if (writeable) {
                f->file_index = fs_find_file(fs_ctx, path);
                if (f->file_index == FS_INVALID_INDEX) {
                    // The file does not exist, create a new one
                    if ((f->file_index = fs_create_file(fs_ctx, path)) == FS_INVALID_INDEX) {
                        //print("sys_fopen: cannot create new file\n");
                        return -1;
                    }
                    if (!fs_write(fs_ctx, f->file_index, NULL, 0, 0)) {
                        //print("sys_fopen: cannot write empty file\n");
                        return -1;
                    }
                }
            } else {
                if ((f->file_index = fs_find_file(fs_ctx, path)) == FS_INVALID_INDEX) {
                    //print("sys_open: cannot open existing file\n");
                    return -1;
                }
            }

            f->position = 0;

            return 3 + i;
        }
    }

    //print("sys_fopen: maximum number of open files reached\n");

    return -1;
}

int sys_fclose(int file_handle) {
    //print("sys_fclose\n");

    // If the file handle is a standard steanm, return immediately
    if (file_handle < 3)
        return 0;

    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    file_t* f = &bios_globals->files[file_handle - 3];

    // If the file is already close, return immediately
    if (f->file_index == FS_INVALID_INDEX)
        return 0;

    fs_context_t* fs_ctx = &bios_globals->fs_ctx;

    fs_sync(fs_ctx);

    f->file_index = FS_INVALID_INDEX;
    //print("sys_close: success\n");
    return 0;
}

int sys_fread(int handle, void* buffer, unsigned size) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;

    if (handle > 2) {
        //print("sys_fread called\n");

        file_t* f = &bios_globals->files[handle - 3];

        fs_context_t* fs_ctx = &bios_globals->fs_ctx;
        size_t nb_read_bytes;
        if (!fs_read(fs_ctx, f->file_index, buffer, f->position, size, &nb_read_bytes))
            return 0;
        f->position += nb_read_bytes;
        return nb_read_bytes;
    } else {
        // stdin
        if (size >= 1) {
            char c = getchar(0);    // non-blocking
            if (c) {
                if (c == 13)
                    c = 10;
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
        //print("sys_fwrite called\n");

        file_t* f = &bios_globals->files[handle - 3];
        fs_context_t* fs_ctx = &bios_globals->fs_ctx;
        if (!fs_write(fs_ctx, f->file_index, buffer, f->position, size))
            return 0;
        f->position += size;
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

int sys_fseek(int handle, int base, unsigned offset_low, int offset_high) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    
    if (handle > 2) {

        //print("sys_fseek\n");

        if (base > 2)
            return -1;

        // Sanity check (we don't support negative values yet)
        if (offset_high != 0) {
            print("sys_fseek: offset_high not zero (negative?)\nSystem halted\n");
            for(;;);
        }

        file_t* f = &bios_globals->files[handle - 3];
        

        // set new position
        fs_context_t* fs_ctx = &bios_globals->fs_ctx;
        fs_file_info_t file_info;
        if (!fs_get_file_info(fs_ctx, f->file_index, &file_info, true)) {
            //print("sys_fseek: get file info failed\n");
            return -1;
        }
        f->position = (base == 0) ? offset_low : (base == 1) ? f->position + offset_low : file_info.size + offset_low;


        return 0;
    };

    return -1;
}

int sys_ftell(int handle, unsigned position[2]) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;

    if (handle > 2) {
        file_t* f = &bios_globals->files[handle - 3];
        position[0] = f->position;
        position[1] = 0;
    }
    return 0;
}

int sys_ftrunc(int handle, unsigned size_low, unsigned size_high) {
        bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;

    if (handle > 2) {
        //print("sys_ftrunc called\n");
        file_t* f = &bios_globals->files[handle - 3]; 
        fs_context_t* fs_ctx = &bios_globals->fs_ctx;
        if (!fs_write(fs_ctx, f->file_index, (void*)0, size_low, 0)) {
            //print("sys_ftrunc: Unable to write\n");
            return -1;
        }
        return 0;
    }
    return -1;
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
    if (!fs_get_file_info(fs_ctx, bios_globals->dir_file_index, &file_info, false))
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
        if (!fs_get_file_info(fs_ctx, i, &file_info, false))
            break;
        if (strcmp(file_info.name, path) == 0) {
            output[0] = 0;              // type = file
            output[1] = 493;            // mode = executable
            output[2] = file_info.size; // size_low
            output[3] = 0;              // size_high
            found = true;
            break;
        }
    }
    return found ? 0 : 1;
}

int sys_rename(const char* from, const char* to) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    fs_context_t* fs_ctx = &bios_globals->fs_ctx;

    if (!fs_rename(fs_ctx, from, to))
        return 1;

    return 0;
}

int sys_unlink(const char* path) {
    bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
    fs_context_t* fs_ctx = &bios_globals->fs_ctx;
    
    if (fs_delete(fs_ctx, path))
        return 0;

    return -1;
}

int sys_mkdir(const char* path) {
    return 0;
}

int sys_missing(unsigned int call_number) {
    print("System call not implemented: ");
    printv(call_number, 10);
    print("\nSystem halted\n");
    for(;;);
}