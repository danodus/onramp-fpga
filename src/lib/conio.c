// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "conio.h"
#include "vdu.h"

#define DEBUG 0

#if DEBUG
#include <stdio.h>

#define UART_DATA   0x22000000
#define UART_STATUS 0x22000004
#define UART_TX_READY 0x2

static void putchar(char c) {
    while((*((int *)UART_STATUS) & UART_TX_READY) == 0);
    *((int *)UART_DATA) = c;
}

static void print(const char* s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}
#endif

static void update_vdu_cursor(conio_context_t* ctx) {
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
#if DEBUG
    print("------------\nconio_init\n");
#endif
    kbd_init(&ctx->kbd_ctx);
    ctx->kbd_last_char = 0;
    ctx->td.state = ANSI_STATE_NORMAL;

    conio_clrscr(ctx);
    *((int *)(VDU_CURSOR_ON)) = 0x1;
}

void conio_clrscr(conio_context_t* ctx) {
    int* fb;
    int i;
    fb = (int *)VDU_FB;
    for (i = 0; i < VDU_SCREEN_WIDTH * VDU_SCREEN_HEIGHT / 2; ++i) {
        *fb = 0x0F000F00;
        fb++;
    }
    ctx->curpos = 0;
    update_vdu_cursor(ctx);
}

static void clear(int pstart, int pend) {
    unsigned short* fb = (unsigned short *)VDU_FB + pstart;
    unsigned short* fb_end = fb + pend;
    while (fb != fb_end)
        *fb++ = 0x0F00;
}

static void ansi_begin_csi_or_esc(conio_context_t* ctx, char cdata) {
#if DEBUG    
    print("ansi_begin_csi_or_esc\n");
#endif
    ansiterm_data_t* td = &ctx->td;
    td->state = (cdata == '\x1b') ? ANSI_STATE_ESC : ANSI_STATE_CSI;
    td->intermediate_char = 0;
    td->num_parms = 0;
    for (int i = 0; i < MAX_CSI_PARMS; ++i)
        td->csi_parms[i] = 0;
}

static void ansi_process_csi(conio_context_t* ctx, char cdata) {
#if DEBUG
    print("ansi_process_csi\n");
    char buf[64];
    sprintf(buf, " %c\n", cdata);
    print(buf);
#endif

    ansiterm_data_t* td = &ctx->td;
    unsigned short int num_z = td->csi_parms[0];
    unsigned short int num = num_z ? num_z : 1;

    td->state = ANSI_STATE_NORMAL;
    switch (cdata) {
        case 'H':
            // VT: <CSI><row>;<col>H    CUP cursor home / position
            {
                int x = td->csi_parms[1] ? td->csi_parms[1] - 1 : 0;
                int y = td->csi_parms[0] ? td->csi_parms[0] - 1 : 0;
                if (x >= VDU_SCREEN_WIDTH)
                    x = VDU_SCREEN_WIDTH - 1;
                if (y >= VDU_SCREEN_HEIGHT)
                    y = VDU_SCREEN_HEIGHT - 1;
                ctx->curpos = y * VDU_SCREEN_WIDTH + x;
#if DEBUG                
                sprintf(buf, " %d,%d\n", x, y);
                print(buf);
#endif
                update_vdu_cursor(ctx);
            }
            break;
        case 'h':
        case 'l':
            if (td->intermediate_char == '?') {
                if (num == 25) {
                    if (cdata == 'l') {
                        // hide cursor
                        *(int *)(VDU_CURSOR_ON) = 0x0;
                    } else {
                        // show cursor
                        *(int *)(VDU_CURSOR_ON) = 0x1;
                    }
                }
            }
            break;
        case 'K':
            // VT:  <CSI>K  EL  erase from cursor to end of line
            {
                int pend = ctx->curpos + VDU_SCREEN_WIDTH;
                pend -= pend - VDU_SCREEN_WIDTH;

                clear(ctx->curpos, pend);
            }
            break;
    }

}

static void ansi_process_char(conio_context_t* ctx, char cdata) {
    ansiterm_data_t* td = &ctx->td;

    char *fb;
    
    if (cdata == '\r') {
        ctx->curpos -= ctx->curpos % VDU_SCREEN_WIDTH;
    } else if (cdata == '\n') {
        ctx->curpos += VDU_SCREEN_WIDTH;
        ctx->curpos -= ctx->curpos % VDU_SCREEN_WIDTH;
    } else if (cdata == '\b') {
        if (ctx->curpos > 0)
            ctx->curpos--;
    } else if (cdata == '\t') {
        ctx->curpos += 8;
        ctx->curpos -= ctx->curpos % 8;
    } else if (cdata != 127) {
        fb = (char *)VDU_FB;
        fb += ctx->curpos << 1;
        fb[0] = cdata;
        ctx->curpos++;
    }

    if (ctx->curpos >= VDU_SCREEN_WIDTH * VDU_SCREEN_HEIGHT) {
        scroll_up(ctx);
        ctx->curpos = VDU_SCREEN_WIDTH * (VDU_SCREEN_HEIGHT - 1);
    }

    update_vdu_cursor(ctx);
}

static void ansi_process_esc(conio_context_t* ctx, char cdata) {
#if DEBUG
    print("ansi_process_esc\n");
    char buf[32];
    sprintf(buf, " %c\n", cdata);
    print(buf);
#endif

    ansiterm_data_t* td = &ctx->td;

    td->state = ANSI_STATE_NORMAL;
    switch (cdata) {
        case '[':
            // VT: <ESC>[ CSI
            ansi_begin_csi_or_esc(ctx, cdata);
            return;
    }
}

static void ansi_parse_csi(conio_context_t* ctx, char cdata) {
#if DEBUG
    print("ansi_parse_csi\n");
    char buf[32];
    sprintf(buf, " %c\n", cdata);
    print(buf);
#endif

    ansiterm_data_t* td = &ctx->td;

    unsigned char cclass = cdata & 0xf0;

    if (cdata <= ' ' || cdata == 0x7f) {
        // ignored
        return;
    } else if (cclass == 0x20) {
        // intermediate char
#if DEBUG
        print(" intermediate char\n");
#endif
        td->intermediate_char = cdata;
    } else if (cclass == 0x30) {
        // parameter number
#if DEBUG
        print(" parameter number\n");
#endif
        unsigned char d = (unsigned char)(cdata - '0');
        if (d <= 9) {
            if (td->num_parms == 0)
                td->num_parms = 1;
            unsigned short v = td->csi_parms[td->num_parms - 1];
            v *= (unsigned short)10;
            if ((unsigned short)(v + d) < v) {
                v = 65535;
            } else {
                v += d;
            }
            td->csi_parms[td->num_parms - 1] = v;
        } else if (cdata == ';') {
            if (td->num_parms < MAX_CSI_PARMS - 1)
                td->num_parms++;
        } else {
            td->intermediate_char = cdata;
        }
    } else if (cclass >= 0x40) {
        ansi_process_csi(ctx, cdata);
    } else {
        // illegal state
    }
}

void conio_putch(conio_context_t* ctx, char c) {

    ansiterm_data_t* td = &ctx->td;

    // ESC or 8-bit CSI received
    if ((c & 0x7f) == '\x1b') {
        // start new CSI/ESC
        ansi_begin_csi_or_esc(ctx, c);
        return;
    } 

    if (td->state == ANSI_STATE_NORMAL) {
        ansi_process_char(ctx, c);
    } else if (ctx->td.state == ANSI_STATE_ESC) {
        ansi_process_esc(ctx, c);
    } else if (ctx->td.state == ANSI_STATE_CSI) {
        ansi_parse_csi(ctx, c);
    }
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
