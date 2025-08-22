// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

static void putchar(char c) {
    *(int *)(0x20000004) = c;
}

static void print(char *s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}

int main(void) {
    print("OS: Hello, world!\r\n");
    return 0;
}