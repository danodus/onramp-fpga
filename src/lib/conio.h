// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef CONIO_H
#define CONIO_H

#include "kbd.h"

typedef struct {
    kbd_context_t kbd_ctx;
    int curpos;
    int kbd_last_char;
} conio_context_t;

void conio_init(conio_context_t* ctx);
void conio_clrscr(conio_context_t* ctx);
void conio_putch(conio_context_t* ctx, char c);
int conio_kbhit(conio_context_t* ctx);
int conio_getch(conio_context_t* ctx);
int conio_getche(conio_context_t* ctx);

#endif
