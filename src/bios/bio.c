// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Based on XV6 (RISC-V)
// Copyright (c) 2006-2024 Frans Kaashoek, Robert Morris, Russ Cox,
//                         Massachusetts Institute of Technology

#ifndef BIO_H
#define BIO_H

#include "globals.h"

#include "stdint.h"
#include "fs.h"
#include "sdc.h"
#include "buf.h"

// blocks per sector
#define BPS (BSIZE / SSIZE)

// Return a locked buf with the contents of the indicated block.
struct buf* bread(uint32_t dev, uint32_t blockno) {
    bios_globals_t* g = (bios_globals_t*)BIOS_GLOBALS;

    uint8_t* p = g->buf.data;

    for (uint32_t i = 0; i < BPS; ++i) {
        if (!sdc_read_sector(blockno * BPS + i, p))
            panic("sdc_read_sector");
        p += SSIZE;
    }

    return &g->buf;
}

#endif
