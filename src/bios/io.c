// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "io.h"

#define CFG         0x20000000

#define SIM_TX      0x20000004
#define SIM_RX      0x20000008

#define LED         0x20001000

#define UART_DATA   0x20002000
#define UART_STATUS 0x20002004

#define UART_RX_READY 0x1
#define UART_TX_READY 0x2

int is_hardware(void) {
    return *(int *)(CFG) & 1;
}

void putchar(char c) {
    if (is_hardware()) {
        while((*((int *)UART_STATUS) & UART_TX_READY) == 0);
        *((int *)UART_DATA) = c;
    } else {
        *(int *)(SIM_TX) = c;
    }
}

char getchar(int blocking) {
    char c;
    if (is_hardware()) {
        int rx_ready;
        do {
            rx_ready = (*((int *)UART_STATUS) & UART_RX_READY);
        } while (!rx_ready && blocking);
        return rx_ready ? *((int *)UART_DATA) : 0;
    } else {
        do {
            c = *(int *)(SIM_RX);
        } while (c == 0 && blocking);
    }
    return c;
}

unsigned int receive_word() {
    unsigned int word = 0;
    for (int i = 0; i < 4; ++i) {
        word <<= 8;
        int rx_ready;
        do {
            rx_ready = (*((int *)UART_STATUS) & UART_RX_READY);
        } while (!rx_ready);        
        unsigned int c = *((unsigned int *)UART_DATA);
        word |= c;
    }
    return word;
}

void set_led(int value) {
    if (is_hardware()) {
         *(int *)(LED) = value;
    }
}