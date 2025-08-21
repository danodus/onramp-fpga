// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "io.h"

static void print(char *s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}

int main(int argc, char *argv[]) {
    for (;;)
        print("Hello, world!\r\n");
    return 0;
}