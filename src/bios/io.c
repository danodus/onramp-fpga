// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "io.h"

void putchar(char c) {
    *(int *)(0x20000004) = c;
}

char getchar(int blocking) {
    char c;
    do {
        c = *(int *)(0x20000008);
    } while (c == 0 && blocking);
    return c;
}
