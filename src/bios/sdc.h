// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef SDC_H
#define SDC_H

#include "common.h"

#define SSIZE  512

bool sdc_init(void);
void sdc_dispose(void);

bool sdc_read_sector(uint32_t addr, uint8_t* buf);
bool sdc_write_sector(uint32_t addr, const uint8_t* buf);

#endif
