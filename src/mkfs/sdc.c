// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "sdc.h"

#include <stddef.h>
#include <stdio.h>

static FILE* f = NULL;

bool sdc_init(const char *img_path)
{
    f = fopen(img_path, "w+b");
    if (f == NULL)
        return false;
    return true;
}

void sdc_dispose(void)
{
    if (f != NULL) {
        fclose(f);
        f = NULL;
    }
}

bool sdc_read_single_block(uint32_t addr, uint8_t* buf)
{
    if (fseek(f, addr, SEEK_SET) != 0) {
        printf("fseek failed\n");
        return false;
    }
    if (fread(buf, SDC_BLOCK_LEN, 1, f) != 1) {
        printf("Unable to read SD block\n");
        return false;
    }
    return true;
}

bool sdc_write_single_block(uint32_t addr, const uint8_t *buf)
{
    if (fseek(f, addr, SEEK_SET) != 0) {
        printf("fseek failed\n");
        return false;
    }
    if (fwrite(buf, SDC_BLOCK_LEN, 1, f) != 1) {
        printf("Unable to write SD block\n");
        return false;
    }
    return true;
}
