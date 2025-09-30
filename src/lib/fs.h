// Copyright (c) 2022-2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef FS_H
#define FS_H

#include <sdc.h>

#define FS_MAX_FILENAME_LEN 63      // maximum filename length (excluding the terminating null byte)
#define FS_MAX_NB_FILES     512
#define FS_MAX_NB_BLOCKS    (6*1024*1024 / SDC_BLOCK_LEN)       // maximum number of blocks supported by the FS

#define FS_INVALID_INDEX    0xFFFF   

typedef struct {
    char name[FS_MAX_FILENAME_LEN + 1];     // entry not set if name begins with '\0'
    uint32_t size;                          // size in bytes
    uint32_t first_block_table_index;       // zero if file is empty
} fs_file_info_t;

typedef struct {
    uint8_t magic[2];                               // "FS"
    fs_file_info_t file_infos[FS_MAX_NB_FILES];     // file information entries
    uint16_t blocks[FS_MAX_NB_BLOCKS];              // block table
} fs_fat_t;

typedef struct {
    bool is_dirty;
    fs_fat_t fat, tmp_fat;
} fs_context_t;

bool fs_format(fs_context_t* ctx, bool quick);

bool fs_mount(fs_context_t* ctx);
uint16_t fs_get_nb_files(fs_context_t* ctx);
bool fs_get_file_info(fs_context_t* ctx, uint16_t file_index, fs_file_info_t* file_info);
bool fs_read(fs_context_t* ctx, uint16_t file_index, uint8_t* buf, size_t current_pos, size_t nb_bytes, size_t* nb_read_bytes);
bool fs_write(fs_context_t* ctx, uint16_t file_index, const uint8_t* buf, size_t current_pos, size_t nb_bytes);
bool fs_delete(fs_context_t* ctx, const char* filename);
bool fs_rename(fs_context_t* ctx, const char* filename, const char* new_filename);
bool fs_file_exists(fs_context_t* ctx, const char* filename);
size_t fs_get_file_size(fs_context_t* ctx, const char* filename);
bool fs_sync(fs_context_t* ctx);
uint16_t fs_find_file(fs_context_t* ctx, const char* filename);
uint16_t fs_create_file(fs_context_t* ctx, const char* filename);

#endif
