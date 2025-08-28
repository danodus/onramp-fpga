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
    *(uint32_t *)(0x2000000C) = addr * SDC_BLOCK_LEN;
    for (size_t i = 0; i < SDC_BLOCK_LEN; i++) {
        *buf = *(uint32_t *)(0x20000010);
        buf++;
    }

    return true;
}

bool sdc_write_single_block(uint32_t addr, const uint8_t* buf) {
    *(uint32_t *)(0x2000000C) = addr * SDC_BLOCK_LEN;
    for (size_t i = 0; i < SDC_BLOCK_LEN; i++) {
        *(uint32_t *)(0x20000010) = *buf;
        buf++;
    }

    return true;
}
