// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef SDC_H
#define SDC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SDC_BLOCK_LEN  512

bool sdc_init(const char* img_path);
void sdc_dispose(void);

bool sdc_read_single_block(uint32_t addr, uint8_t* buf);
bool sdc_write_single_block(uint32_t addr, const uint8_t* buf);

#endif
