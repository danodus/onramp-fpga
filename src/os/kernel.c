// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void set_led(int value) {
    *(int *)(0x2000F000) = value;
}

void exit_handler(void) {
    set_led(0x55);
}

int main(void) {
    set_led(0);

    atexit(exit_handler);

    for (int counter = 0; counter < 10; counter++) {
        printf("OS: Hello using system calls! [%02d/10]\r\n", counter);
        usleep(50000);
        // write to LEDs
        set_led(counter + 1);
    }

    return 42;
}