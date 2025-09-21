// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include "conio.h"

#include <unistd.h>

char* read_line(char *s, size_t len) {
    int n;
    char c;
    size_t i = 0;
    s[0] = '\0';
    for (;;) {
        n = read(0, &c, 1);
        if (n < 0)
            break;
        if (n == 1) {
            if (c == 10) {
                if (i < len) {
                    s[i] = '\n';
                    i++;
                    s[i] = '\0';
                }
                break;
            } else if (c == 127) {
                // backspace
                if (i > 0) {
                    i--;
                    s[i] = '\0';
                    c = 8;
                    write(1, &c, 1);
                    c = 32;
                    write(1, &c, 1);
                    c = 8;
                    write(1, &c, 1);
                }
            } else {
                if (i < len) {
                    s[i] = c;
                    i++;
                    s[i] = '\0';
                }
            }
        }
    }
    return s;
}
