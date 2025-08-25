// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "system.h"

static void sysprint(const char *s) {
    while (*s)
        if (write(1, s++, 1) < 0)
            return;
}

int main(void) {
    sysprint("OS: Hello using system calls!\r\n");
    return 0x55;
}