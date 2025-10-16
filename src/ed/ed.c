// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

/*** includes ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <termios.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

typedef struct {
    int screen_rows;
    int screen_cols;

} editor_config_t;

editor_config_t editor_config;

/*** terminal ***/

void die(const char* s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}

char editor_read_key() {
    char c;
    while (read(STDIN_FILENO, &c, 1) != 1);
    return c;
}

int get_cursor_position(int* rows, int* cols) {
    char buf[32];
    char* t;
    unsigned int i = 0;

    write(STDOUT_FILENO, "\x1b[6n", 4);

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    t = strtok(buf + 2, ";");
    if (t == NULL)
        return -1;
    *rows = atoi(t);
    t = strtok(NULL, "");
    if (t == NULL)
        return -1;
    *cols = atoi(t);
    return 0;
}

int get_window_size(int* rows, int* cols) {
    write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12);
    return get_cursor_position(rows, cols);
}

/*** output ***/

void editor_draw_rows() {
    int y;
    for (y = 0; y < editor_config.screen_rows; y++) {
        write(STDOUT_FILENO, "~", 1);
        if (y < editor_config.screen_rows - 1)
            write(STDOUT_FILENO, "\r\n", 2);
    }
}

void editor_refresh_screen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    editor_draw_rows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editor_process_keypress() {
    char c = editor_read_key();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

/*** init ***/

void init_editor() {
    if (get_window_size(&editor_config.screen_rows, &editor_config.screen_cols) == -1)
        die("getWindowSize");
}

int main(void) {
    enable_raw_mode();
    init_editor();

    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
    return 0;
}