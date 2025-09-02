// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef IO_H
#define IO_H

int is_hardware(void);
void set_led(int value);

void putchar(char c);
char getchar(int blocking);

unsigned int receive_word(void);

#endif
