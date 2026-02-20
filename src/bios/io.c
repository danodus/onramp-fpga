// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "io.h"

#include <conio.h>
#include <globals.h>

#define CFG         0x20000000

#define SIM_TX      0x20000004
#define SIM_RX      0x20000008

#define LED         0x21000000

#define UART_DATA   0x22000000
#define UART_STATUS 0x22000004

#define UART_RX_READY 0x1
#define UART_TX_READY 0x2

int is_hardware(void) {
    return *(int *)(CFG) & 1;
}

void putchar(char c) {
    if (is_hardware()) {
        bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
        // while((*((int *)UART_STATUS) & UART_TX_READY) == 0);
        // *((int *)UART_DATA) = c;
        conio_putch(&bios_globals->conio_ctx, c);
    } else {
        *(int *)(SIM_TX) = c;
    }
}

char getchar(int blocking) {
    char c;
    if (is_hardware()) {
        bios_globals_t* bios_globals = (bios_globals_t*)BIOS_GLOBALS;
        int rx_ready;
        do {
            // rx_ready = (*((int *)UART_STATUS) & UART_RX_READY);
            // if (rx_ready) {
            //     c = *((int *)UART_DATA);
            // } else {
                rx_ready = conio_kbhit(&bios_globals->conio_ctx);
                if (rx_ready)
                    c = conio_getch(&bios_globals->conio_ctx);
            // }
        } while (!rx_ready && blocking);
        return rx_ready ? c : 0;
    } else {
        do {
            c = *(int *)(SIM_RX);
        } while (c == 0 && blocking);
    }
    return c;
}

unsigned char receive_byte(void) {
    while ((*((int *)UART_STATUS) & UART_RX_READY) == 0);
    return *((unsigned int *)UART_DATA);
}

unsigned int receive_word(void) {
    unsigned int word = 0;
    for (int i = 0; i < 4; ++i) {
        word <<= 8;
        word |= (unsigned int)receive_byte();
    }
    return word;
}

void set_led(int value) {
    if (is_hardware()) {
         *(int *)(LED) = value;
    }
}