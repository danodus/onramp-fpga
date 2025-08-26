// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "io.h"

typedef unsigned long size_t;

static void print(char *s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}

int sys_time(unsigned out_buffer[3]) {
    for (size_t i = 0; i < 4; ++i)
        out_buffer[i] = 0;
    return 0;
}

int sys_fwrite(int handle, const void* buffer, unsigned size) {
    for (unsigned i = 0; i < size; ++i) {
        putchar(*((char *)buffer));
        ++buffer;
    }
    return size;
}

int main(void) {
    print("BIOS: Initialized\r\n");
    return 0;
}