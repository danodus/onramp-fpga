// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#ifndef CONIO_H
#define CONIO_H

#include "kbd.h"

#define MAX_CSI_PARMS   16

enum {
    ANSI_STATE_NORMAL,
    ANSI_STATE_ESC,
    ANSI_STATE_CSI
};

typedef struct {
    int state;
    char intermediate_char;
    unsigned short csi_parms[MAX_CSI_PARMS];
    int num_parms;
} ansiterm_data_t;

typedef struct {
    kbd_context_t kbd_ctx;
    int curpos;
    int kbd_last_char;
    ansiterm_data_t td;
} conio_context_t;

void conio_init(conio_context_t* ctx);
void conio_clrscr(conio_context_t* ctx);
void conio_putch(conio_context_t* ctx, char c);
int conio_kbhit(conio_context_t* ctx);
int conio_getch(conio_context_t* ctx);
int conio_getche(conio_context_t* ctx);

#endif
