// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Based on XV6 (RISC-V)
// Copyright (c) 2006-2024 Frans Kaashoek, Robert Morris, Russ Cox,
//                         Massachusetts Institute of Technology

#ifndef BUF_H
#define BUF_H

#include <fs.h>

struct buf {
    uint8_t data[BSIZE];
};

#endif