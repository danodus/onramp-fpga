// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "common.h"

#include "io.h"

int strcmp(const char* a, const char* b) {
    while (*a == *b) {
        if (*a == 0)
            return 0;
        ++a;
        ++b;
    }
    return (int)(unsigned char)(*a) - (int)(unsigned char)(*b);
}

char* strncpy(char* restrict to, const char* restrict from, size_t count) {
    char* ret = to;
    char* end = to + count;
    while (to != end && *from != 0) {
        *to = *from;
        ++to;
        ++from;
    }
    /* strncpy() pads with zeroes. */
    while (to != end)
        *to++ = 0;
    return ret;
}

void print(const char* s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}