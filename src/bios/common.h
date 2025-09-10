// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef COMMON_H
#define COMMON_H

#include "stdint.h"

#define true ((_Bool)1)
#define false ((_Bool)0)
#define bool _Bool

typedef unsigned long size_t;

int strcmp(const char* a, const char* b);
char* strncpy(char* restrict to, const char* restrict from, size_t count);
void* memmove(void* vdest, const void* vsrc, size_t count);
void print(const char* s);

_Noreturn void panic(const char* s);

// fs.c
void fsinit(int dev);
struct inode* namei(const char *path);

// bio.c
struct buf* bread(uint32_t dev, uint32_t blockno);


#endif
