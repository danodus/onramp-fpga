// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "kbd.h"

#define KEYMAP_SIZE 132

void kbd_init(kbd_context_t* ctx) {
    ctx->brk = 0;
    ctx->control = 0;
    ctx->modifier = 0;
    ctx->shift = 0;
}

int kbd_get_char(kbd_context_t* ctx, int is_blocking) {

    const char keymap[] = 
        // Without shift or control
        "             \x09""`      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09"
        "  ./l;p-   \' [=    \x0D""] \\        \x7F""  1 47   0.2568\x1B""  +3-*9      "
        // With shift
        "             \x09""~      Q!   ZSAW@  CXDE$#   VFTR%  NBHGY^   MJU&*  <KIO)("
        "  >?L:P_   \" {+    \x0D""} |        \x7F""  1 47   0.2568\x1B""  +3-*9      "
        // With control
        "             \x09""`      \x11""1   \x1A""\x13""\x01""\x17""2  \x03""\x18""\x04""\x05""43  \x00""\x16""\x06""\x14""\x12""5  \x0E""\x02""\x7F""\x07""\x19""6   \x0D""\x0A""\x15""78  ,\x0B""\x09""\x0F""09"
        "  ./\x0C"";\x10""-   \' \x1B""=    \x0D""\x1D"" \x1C""        \x7F""  1 47   0.2568\x1B""  +3-*9      "
        // With control and shift
        "             \x09""`      \x11""1   \x1A""\x13""\x01""\x17""\x00""  \x03""\x18""\x04""\x05""43   \x16""\x06""\x14""\x12""5  \x0E""\x02""\x7F""\x07""\x19""\x1E""   \x0D""\x0A""\x15""78  ,\x0B""\x09""\x0F""09"
        "  ./\x0C"";\x10""\x1F""   \' [=    \x0D""] \\        \x7F""  1 47   0.2568\x1B""  +3-*9      ";

    int status, code, c;

    for (;;) {
        // if character available
        status = *(int *)(PS2_KBD_STATUS);
        if (status & 0x1) {
            // read character
            code = *(int *)(PS2_KBD_DATA);

            if (code == 0xAA) { // BAT completion code
                continue; 
            }
            if (code == 0xF0) {
                ctx->brk = 1;
                continue;
            }
            if (code == 0xE0) {
                ctx->modifier = 1;
                continue;
            }
            if (ctx->brk) {
                if ((code == 0x12) || (code == 0x59)) {
                    ctx->shift = 0;
                } else if (code == 0x14)
                    ctx->control = 0;
                ctx->brk = 0;
                ctx->modifier = 0;
                continue;
            }
            if ((code == 0x12) || (code == 0x59)) {
                // left of right shift
                ctx->shift = 1;
                ctx->brk = 0;
                ctx->modifier = 0;
                continue;
            }
            if (code == 0x14) {
                // left or right control
                ctx->control = 1;
                ctx->brk = 0;
                ctx->modifier = 0;
                continue;
            }
            
            if (ctx->modifier)
                return 0x8000 | code;

            if (ctx->control)
                return 0x8200 | code;

            // function keys
            if (code == 0x05 ||
                code == 0x06 ||
                code == 0x04 ||
                code == 0x0C ||
                code == 0x03 ||
                code == 0x0B ||
                code == 0x83 ||
                code == 0x0A ||
                code == 0x01 ||
                code == 0x09 ||
                code == 0x78 ||
                code == 0x07 ||
                code == 0x7E)
                return 0x8100 | code;

            c = 0;
            if (code < KEYMAP_SIZE)
                c = keymap[code + KEYMAP_SIZE * ctx->shift];
            return c;
        } else {
            if (!is_blocking)
                break;
        }
    }

    // no character available
    return 0;
}
