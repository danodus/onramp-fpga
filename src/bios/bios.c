// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "io.h"

static void print(char *s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}

int sys_fwrite(int handle, const void* buffer, unsigned size) {
    if (size > 0)
        putchar(((char *)buffer)[0]);
    return 1;
}

int main(void) {
    print("BIOS: Initialized\r\n");
    return 0;
}