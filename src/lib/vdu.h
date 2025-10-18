// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef VDU_H
#define VDU_H

#define VDU_FB 0x25000000
#define VDU_REGS 0x25010000
#define VDU_CURSOR_ON (VDU_REGS + 0x0)
#define VDU_CURSOR_POS (VDU_REGS + 0x4)

#define VDU_SCREEN_WIDTH    80
#define VDU_SCREEN_HEIGHT   60

#endif
