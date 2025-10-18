// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "conio.h"

#include "vdu.h"

static void update_vdu_cursor(conio_context_t* ctx) {
    *((int *)(VDU_CURSOR_ON)) = 0x1;
    *((int *)(VDU_CURSOR_POS)) = ctx->curpos;
}

static void scroll_up(conio_context_t* ctx) {
    int *fb;
    int i;
    fb = (int *)(VDU_FB);
    for (i = 0; i < VDU_SCREEN_WIDTH * (VDU_SCREEN_HEIGHT - 1) / 2; ++i) {
        *fb = *(fb + VDU_SCREEN_WIDTH / 2);
        fb++;
    }
    for (i = 0; i < VDU_SCREEN_WIDTH / 2; ++i) {
        *fb = 0x0F000F00;
        fb++;
    }
}

void conio_init(conio_context_t* ctx) {
    kbd_init(&ctx->kbd_ctx);
    ctx->kbd_last_char = 0;
    conio_clrscr(ctx);
}

void conio_clrscr(conio_context_t* ctx) {
    int *fb;
    int i;
    fb = (int *)VDU_FB;
    for (i = 0; i < VDU_SCREEN_WIDTH * VDU_SCREEN_HEIGHT / 2; ++i) {
        *fb = 0x0F000F00;
        fb++;
    }
    ctx->curpos = 0;
    update_vdu_cursor(ctx);
}

void conio_putch(conio_context_t* ctx, char c) {
    char *fb;
    if (c == '\n') {
        ctx->curpos += VDU_SCREEN_WIDTH;
        ctx->curpos -= ctx->curpos % VDU_SCREEN_WIDTH;
    } else if (c == '\b') {
        if (ctx->curpos > 0)
            ctx->curpos--;
    } else if (c == '\t') {
        ctx->curpos += 8;
        ctx->curpos -= ctx->curpos % 8;
    } else if (c != 127) {
        fb = (char *)VDU_FB;
        fb += ctx->curpos << 1;
        fb[0] = c;
        ctx->curpos++;
    }

    if (ctx->curpos >= VDU_SCREEN_WIDTH * VDU_SCREEN_HEIGHT) {
        scroll_up(ctx);
        ctx->curpos = VDU_SCREEN_WIDTH * (VDU_SCREEN_HEIGHT - 1);
    }

    update_vdu_cursor(ctx);
}

int conio_kbhit(conio_context_t* ctx) {
    if (ctx->kbd_last_char)
        return 1;
    ctx->kbd_last_char = kbd_get_char(&ctx->kbd_ctx, 0);
    return ctx->kbd_last_char != 0;
}

int conio_getch(conio_context_t* ctx) {
    int c;
    while (!conio_kbhit(ctx));
    c = ctx->kbd_last_char;
    ctx->kbd_last_char = 0;
    return c;
}

int conio_getche(conio_context_t* ctx) {
    int c;
    while (!conio_kbhit(ctx));
    c = ctx->kbd_last_char;
    ctx->kbd_last_char = 0;
    conio_putch(ctx, c);
    return c;
}
