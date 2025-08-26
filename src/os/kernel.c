// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>

void exit_handler(void) {
   // Exit the simulation with a recognizable LED pattern
    *(int *)(0x20000000) = 0x55;
}

int main(void) {
    atexit(exit_handler);
    printf("OS: Hello using system calls!\r\n");
    return 0;
}