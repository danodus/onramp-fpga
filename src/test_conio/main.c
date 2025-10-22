// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <conio.h>

char digits[] = "0123456789abcdef";

conio_context_t ctx;

static void printc(int c)
{
    conio_putch(&ctx, c);
}

static void printdec(int xx, int sign)
{
    char buf[16];
    int i;
    int x;

    if (sign && (sign = xx < 0))
        x = -xx;
    else
        x = xx;

    i = 0;
    do {
        buf[i++] = digits[x % 10];
        x = x / 10;
    } while (x != 0);

    if (sign)
        buf[i++] = '-';

    while(--i >= 0)
        printc(buf[i]);
}

static void printhex(int x)
{
    int i;
    for (i = 0; i < 8; i++) {
        printc(digits[(x >> 28) & 0xF]);
        x <<= 4;
    }
}

static int con_printf(char *fmt, ...)
{
    void **varg;
    int i, c;
    char *s;

    varg = (void **) &fmt + 1;

    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        if (c != '%') {
            printc(c);
            continue;
        }
        c = fmt[++i] & 0xff;
        if (c == 0)
            break;
        switch (c) {
        case 'd':
            printdec((int) *varg++, 1);
            break;
        case 'x':
            printhex((int) *varg++);
            break;
        case 'c':
            printc((char) *varg++);
            break;			
        case 's':
            if((s = (char *) *varg++) == (void *)0)
                s = "(null)";
            for(; *s; s++)
                printc(*s);
            break;			
        default:
            printc('%');
            printc(c);
            break;
        }
    }
}

int main(int argc, char *argv[]) {

    conio_init(&ctx);
/*
    // make cursor invisible
    con_printf("\x1b[?25l");
    con_printf("Hide cursor\n");
    conio_getch(&ctx);

    // make cursor visible
    con_printf("\x1b[?25h");
    con_printf("Show cursor\n");
    conio_getch(&ctx);

    // move cursor to home
    con_printf("\x1b[H");
    con_printf("Move cursor to home\n");
    conio_getch(&ctx);

    // move cursor to 10,20
    con_printf("\x1b[10;20H");
    con_printf("Move cursor to 10,20\n");
    conio_getch(&ctx);
*/
    con_printf("\x1b[10;1HFirst Line\r\n");
    con_printf("\x1b[11;1HSecond Line\r\n");
    conio_getch(&ctx);

    con_printf("\x1b[10;1H\x1b[K");
    conio_getch(&ctx);

    con_printf("Done\n");
    return 0;
}