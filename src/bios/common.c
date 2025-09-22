// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "common.h"

#include "io.h"

void* memcpy(void* vdest, const void* vsrc, size_t count) {
    void* start = vdest;
    const unsigned char* src = (const unsigned char*)vsrc;
    unsigned char* dest = (unsigned char*)vdest;
    unsigned char* end = dest + count;
    while (dest != end)
        *dest++ = *src++;
    return start;
}

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

unsigned long clock(void) {
    unsigned long ms;
    ms = *(unsigned int *)(0x30000004);
    ms <<= 32;
    ms |= *(unsigned int *)(0x30000000);
    return ms;
}