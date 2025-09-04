// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef COMMON_H
#define COMMON_H

#define true ((_Bool)1)
#define false ((_Bool)0)
#define bool _Bool

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long size_t;

int strcmp(const char* a, const char* b);
char* strncpy(char* restrict to, const char* restrict from, size_t count);
void print(const char* s);

#endif
