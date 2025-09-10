// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef GLOBALS_H
#define GLOBALS_H

#include <fs.h>
#include "buf.h"

#define BIOS_GLOBALS 0x1003f000

#define MAX_OPEN_FILES  4

typedef struct {
    struct buf buf;

    // there should be one superblock per disk device, but we run with
    // only one device
    struct superblock sb;

} bios_globals_t;

#endif
