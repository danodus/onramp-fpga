// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef _SYSTEM_H
#define _SYSTEM_H

#define NULL (void*)0

typedef unsigned long size_t;
typedef long ssize_t;

char* uitoa(unsigned int value, char* result, int base);
void putchar(char c);
void print(char *s);

void* calloc(unsigned int count, unsigned int element_size);

ssize_t write(int fd, const void* buffer, size_t count);


#endif
