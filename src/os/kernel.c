// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// ref: https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
static char *uitoa(unsigned int value, char* result, int base)
{
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    for (int i = 0; i < 8; ++i) {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    }

    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

static void putchar(char c) {
    *(int *)(0x20000004) = c;
}

static void print(char *s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}

static void exit(int code) {
    *(int *)(0x20000000) = code;
}

int main(void) {
    print("OS: Hello, world!\r\n");

    char buf[32];
    int v = 42/8;
    uitoa(v, buf, 10);
    print(buf);
    print("\r\n");
    exit(v);
    return 0;
}