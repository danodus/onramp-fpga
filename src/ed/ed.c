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

#define ED_VERSION "0.0.1"

#define CTRL_KEY(k) ((k) & 0x1f)

enum editor_key {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

/*** data ***/

typedef struct {
    int cx, cy;
    int screen_rows;
    int screen_cols;
    struct termios orig_termios;
} editor_config_t;

editor_config_t E;

/*** terminal ***/

void write_all(int fd, const void* buffer, size_t count) {
    do {
        ssize_t n = write(fd, buffer, count);
        count -= n;
        buffer += n;
    } while (count > 0);    
}

void die(const char* s) {
    write_all(STDOUT_FILENO, "\x1b[2J", 4);
    write_all(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}

int editor_read_key() {
    char c;
    while (read(STDIN_FILENO, &c, 1) != 1);

    if (c == '\x1b') {
        char seq[3];
        while (read(STDIN_FILENO, &seq[0], 1) != 1);
        while (read(STDIN_FILENO, &seq[1], 1) != 1);

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                while (read(STDIN_FILENO, &seq[2], 1) != 1);
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }

        return '\x1b';
    } else {
        return c;
    }
}

int get_cursor_position(int* rows, int* cols) {
    char buf[32];
    char* t;
    unsigned int i = 0;

    write_all(STDOUT_FILENO, "\x1b[6n", 4);

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
    write_all(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12);
    return get_cursor_position(rows, cols);
}

void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &E.orig_termios) == -1)
        die("tcsetattr");
}

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die("tcgetattr");
    atexit(disable_raw_mode);

    struct termios raw = E.orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

/*** append buffer ***/

typedef struct {
    char *b;
    int len;
} abuf_t;

#define ABUF_INIT {NULL, 0}

void ab_append(abuf_t* ab, const char* s, int len) {
    char* new = realloc(ab->b, ab->len + len);

    if (new == NULL)
        die("ab_append");
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void ab_free(abuf_t* ab) {
    free(ab->b);
}

/*** output ***/

void editor_draw_rows(abuf_t* ab) {
    int y;
    for (y = 0; y < E.screen_rows; y++) {
        if (y == E.screen_rows / 3) {
            char welcome[80];
            int welcome_len = snprintf(welcome, sizeof(welcome), "Onramp-FPGA editor -- version %s", ED_VERSION);
            if (welcome_len > E.screen_cols) welcome_len = E.screen_cols;
            int padding = (E.screen_cols - welcome_len) / 2;
            if (padding) {
                ab_append(ab, "~", 1);
                padding--;
            }
            while (padding--) ab_append(ab, " ", 1);
            ab_append(ab, welcome, welcome_len);
        } else {
            ab_append(ab, "~", 1);
        }

        ab_append(ab, "\x1b[K", 3);
        if (y < E.screen_rows - 1)
            ab_append(ab, "\r\n", 2);
    }
}

void editor_refresh_screen() {
    abuf_t ab = ABUF_INIT;

    ab_append(&ab, "\x1b[?25l", 6);
    ab_append(&ab, "\x1b[H", 3);

    editor_draw_rows(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
    ab_append(&ab, buf, strlen(buf));

    ab_append(&ab, "\x1b[?25h", 6);

    write_all(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

/*** input ***/

void editor_move_cursor(int key) {
    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0)
                E.cx--;
            break;
        case ARROW_RIGHT:
            if (E.cx != E.screen_cols - 1)
                E.cx++;
            break;
        case ARROW_UP:
            if (E.cy != 0)
                E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy != E.screen_rows - 1)
                E.cy++;
            break;
    }
}

void editor_process_keypress() {
    int c = editor_read_key();

    switch (c) {
        case CTRL_KEY('q'):
            write_all(STDOUT_FILENO, "\x1b[2J", 4);
            write_all(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;

        case HOME_KEY:
            E.cx = 0;
            break;

        case END_KEY:
            E.cx = E.screen_cols - 1;
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                int times = E.screen_rows;
                while (times--)
                    editor_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editor_move_cursor(c);
            break;
    }
}

/*** init ***/

void init_editor() {
    E.cx = 0;
    E.cy = 0;

    if (get_window_size(&E.screen_rows, &E.screen_cols) == -1)
        die("get_window_size");
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