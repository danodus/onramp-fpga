// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "sdc.h"

#include <stddef.h>

bool sdc_init(void) {
    return true;
}

void sdc_dispose(void) {
}

bool sdc_read_single_block(uint32_t addr, uint8_t* buf) {
    int i;
    *(uint32_t *)(0x2000000C) = addr;
    for (size_t i = 0; i < SDC_BLOCK_LEN; i++) {
        *buf = *(uint32_t *)(0x20000010);
        buf++;
    }

    return true;
}

bool sdc_write_single_block(uint32_t addr, const uint8_t* buf) {
    *(uint32_t *)(0x2000000C) = addr;
    for (size_t i = 0; i < SDC_BLOCK_LEN; i++) {
        *(uint32_t *)(0x20000010) = *buf;
        buf++;
    }

    return true;
}

bool sdc_read(uint32_t first_block_addr, uint8_t* buf, size_t nb_bytes) {
    size_t remaining_bytes;
    uint32_t block_addr;
    size_t s, i;
    uint8_t b[SDC_BLOCK_LEN];
    remaining_bytes = nb_bytes;
    block_addr = first_block_addr;
    while (remaining_bytes > 0) {
        s = remaining_bytes > SDC_BLOCK_LEN ? SDC_BLOCK_LEN : remaining_bytes;
        if (!sdc_read_single_block(block_addr, b))
            return false;
        for (i = 0; i < s; ++i)
            buf[i] = b[i];
        remaining_bytes -= s;
        block_addr++;
        buf += s;
    }
    return true;
}

bool sdc_write(uint32_t first_block_addr, uint8_t* buf, size_t nb_bytes) {
    size_t remaining_bytes;
    uint32_t block_addr;
    size_t s, i;
    uint8_t b[SDC_BLOCK_LEN];
    remaining_bytes = nb_bytes;
    block_addr = first_block_addr;
    while (remaining_bytes > 0) {
        s = remaining_bytes > SDC_BLOCK_LEN ? SDC_BLOCK_LEN : remaining_bytes;
        for (i = 0; i < SDC_BLOCK_LEN; ++i)
            b[i] = i < s ? buf[i] : 0xFF;
        if (!sdc_write_single_block(block_addr, b))
            return false;
        remaining_bytes -= s;
        block_addr++;
        buf += s;
    }
    return true;
}
