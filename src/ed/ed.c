// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: Build your own text editor booklet (https://viewsourcecode.org/snaptoken/kilo/index.html)

/*** includes ***/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/types.h>

/*** defines ***/

#define ED_VERSION "0.0.1"
#define ED_TAB_STOP 8
#define ED_QUIT_TIMES 3

#define CTRL_KEY(k) ((k) & 0x1f)

enum editor_key {
    BACKSPACE = 127,
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
    int size;
    int rsize;
    char* chars;
    char* render;
} erow_t;

typedef struct {
    int cx, cy;
    int rx;
    int row_off;
    int col_off;
    int screen_rows;
    int screen_cols;
    int num_rows;
    erow_t* rows;
    int dirty;
    char* filename;
    char statusmsg[80];
    time_t statusmsg_time;
    struct termios orig_termios;
} editor_config_t;

editor_config_t E;

/*** prototypes ***/

void editor_set_status_message(const char* fmt, ...);
void editor_refresh_screen();
char* editor_prompt(char* prompt);

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

/*
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
*/

int get_window_size(int* rows, int* cols) {
    // write_all(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12);
    // return get_cursor_position(rows, cols);
    *rows = 60;
    // TODO: Fix issue with 80 cols
    //*cols = 80;
    *cols = 79;
    return 0;
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

/*** libc additions ***/

#define MIN_LINE_SIZE 4
#define DEFAULT_LINE_SIZE 128

ssize_t getdelim(char** bufptr, size_t* n, int delim, FILE* fp) {
    if (fp == NULL || bufptr == NULL || n == NULL)
        return -1;
    char* buf = *bufptr;
    if (buf == NULL || *n < MIN_LINE_SIZE) {
        buf = (char*)realloc(*bufptr, DEFAULT_LINE_SIZE);
        if (buf == NULL)
            return -1;
        *bufptr = buf;
        *n = DEFAULT_LINE_SIZE;
    }

    size_t numbytes = *n;
    char* ptr = buf;
    int cont = 1;
    int ch;
    while (cont) {
        // fill buffer
        while (--numbytes > 0) {
            if ((ch = getc(fp)) == EOF) {
                cont = 0;
                break;
            } else {
                *ptr++ = ch;
                if (ch == delim) {
                    cont = 0;
                    break;
                }
            }
        }

        if (cont) {
            // buffer is too small so reallocate a larger buffer
            int pos = ptr - buf;
            size_t newsize = (*n << 1);
            buf = realloc(buf, newsize);
            if (buf == NULL) {
                cont = 0;
                break;
            }

            // continue in a new buffer
            *bufptr = buf;
            *n = newsize;
            ptr = buf + pos;
            numbytes = newsize - pos;
        }
    }

    // if no input data, return failure
    if (ptr == buf)
        return -1;

    // nul-terminate
    *ptr = '\0';
    return (ssize_t)(ptr - buf);
}

ssize_t getline(char** lptr, size_t* n, FILE* fp) {
    return getdelim(lptr, n, '\n', fp);
}

/*** row operations ***/

int editor_row_cx_to_rx(erow_t* row, int cx) {
    int rx = 0;
    int j;
    for (j = 0; j < cx; j++) {
        if (row->chars[j] == '\t')
            rx += (ED_TAB_STOP - 1) - (rx % ED_TAB_STOP);
        rx++;
    }
    return rx;
}

void editor_update_row(erow_t* row) {
    int tabs = 0;
    int j;
    for (j = 0; j < row->size; j++)
        if (row->chars[j] == '\t') tabs++;;

    free(row->render);
    row->render = malloc(row->size + tabs * (ED_TAB_STOP - 1) + 1);

    int idx = 0;
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % ED_TAB_STOP != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}

void editor_insert_row(int at, char* s, size_t len) {
    if (at < 0 || at > E.num_rows) return;

    E.rows = realloc(E.rows, sizeof(erow_t) * (E.num_rows + 1));
    memmove(&E.rows[at + 1], &E.rows[at], sizeof(erow_t) * (E.num_rows - at));

    E.rows[at].size = len;
    E.rows[at].chars = malloc(len + 1);
    memcpy(E.rows[at].chars, s, len);
    E.rows[at].chars[len] = '\0';

    E.rows[at].rsize = 0;
    E.rows[at].render = NULL;
    editor_update_row(&E.rows[at]);

    E.num_rows++;
    E.dirty = 1;
}

void editor_free_row(erow_t* row) {
    free(row->render);
    free(row->chars);
}

void editor_del_row(int at) {
    if (at < 0 || at >= E.num_rows) return;
    editor_free_row(&E.rows[at]);
    memmove(&E.rows[at], &E.rows[at + 1], sizeof(erow_t) * (E.num_rows - at - 1));
    E.num_rows--;
    E.dirty = 1;
}

void editor_row_insert_char(erow_t* row, int at, int c) {
    if (at < 0 || at > row->size) at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    editor_update_row(row);
    E.dirty = 1;
}

void editor_row_append_string(erow_t* row, char* s, size_t len) {
    row->chars = realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editor_update_row(row);
    E.dirty = 1;
}

void editor_row_del_char(erow_t* row, int at) {
    if (at < 0 || at >= row->size) return;
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    editor_update_row(row);
    E.dirty = 1;
}

/*** editor operations */

void editor_insert_char(int c) {
    if (E.cy == E.num_rows) {
        editor_insert_row(E.num_rows, "", 0);
    }
    editor_row_insert_char(&E.rows[E.cy], E.cx, c);
    E.cx++;
}

void editor_insert_new_line() {
    if (E.cx == 0) {
        editor_insert_row(E.cy, "", 0);
    } else {
        erow_t* row = &E.rows[E.cy];
        editor_insert_row(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.rows[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        editor_update_row(row);
    }
    E.cy++;
    E.cx = 0;
}

void editor_del_char() {
    if (E.cy == E.num_rows) return;
    if (E.cx == 0 && E.cy == 0) return;

    erow_t* row = &E.rows[E.cy];
    if (E.cx > 0) {
        editor_row_del_char(row, E.cx - 1);
        E.cx--;
    } else {
        E.cx = E.rows[E.cy - 1].size;
        editor_row_append_string(&E.rows[E.cy - 1], row->chars, row->size);
        editor_del_row(E.cy);
        E.cy--;
    }
}

/*** file i/o ***/

char* editor_rows_to_string(int* buflen) {
    int totlen = 0;
    int j;
    for (j = 0; j < E.num_rows; j++)
        totlen += E.rows[j].size + 1;
    *buflen = totlen;

    char* buf = malloc(totlen);
    char* p = buf;
    for (j = 0; j < E.num_rows; j++) {
        memcpy(p, E.rows[j].chars, E.rows[j].size);
        p += E.rows[j].size;
        *p = '\n';
        p++;
    }

    return buf;
}

void editor_open(char* filename) {
    free(E.filename);
    E.filename = strdup(filename);
    FILE* fp = fopen(filename, "r");
    if (!fp) die("fopen");

    char* line = NULL;
    size_t line_cap = 0;
    ssize_t line_len;
    while ((line_len = getline(&line, &line_cap, fp)) != -1) {
        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r'))
            line_len--;
        editor_insert_row(E.num_rows, line, line_len);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
}

void editor_save() {
    if (E.filename == NULL) {
        E.filename = editor_prompt("Save as: %s (ESC to cancel)");
        if (E.filename == NULL) {
            editor_set_status_message("Save aborted");
            return;
        }
    }

    int len;
    char* buf = editor_rows_to_string(&len);

    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        write_all(fd, buf, len);
        close(fd);
        free(buf);
        E.dirty = 0;
        editor_set_status_message("%d bytes written to disk", len);
        return;
    }
    free(buf);
    editor_set_status_message("Can't save!");
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

void editor_scroll() {
    E.rx = 0;
    if (E.cy < E.num_rows) {
        E.rx = editor_row_cx_to_rx(&E.rows[E.cy], E.cx);
    }

    if (E.cy < E.row_off)
        E.row_off = E.cy;
    if (E.cy >= E.row_off + E.screen_rows)
        E.row_off = E.cy - E.screen_rows + 1;
    if (E.rx < E.col_off)
        E.col_off = E.rx;
    if (E.rx >= E.col_off + E.screen_cols)
        E.col_off = E.rx - E.screen_cols + 1;
}

void editor_draw_rows(abuf_t* ab) {
    int y;
    for (y = 0; y < E.screen_rows; y++) {
        int file_row = y + E.row_off;
        if (file_row >= E.num_rows) {
            if (E.num_rows == 0 && y == E.screen_rows / 3) {
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
        } else {
            int len = E.rows[file_row].rsize - E.col_off;
            if (len < 0) len = 0;
            if (len > E.screen_cols) len = E.screen_cols;
            ab_append(ab, &E.rows[file_row].render[E.col_off], len);
        }

        ab_append(ab, "\x1b[K", 3);
        ab_append(ab, "\r\n", 2);
    }
}

void editor_draw_status_bar(abuf_t* ab) {
    //ab_append(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
        E.filename ? E.filename : "[No Name]", E.num_rows,
        E.dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
        E.cy + 1, E.num_rows);
    if (len > E.screen_cols) len = E.screen_cols;
    ab_append(ab, status, len);
    while (len < E.screen_cols) {
        if (E.screen_cols - len == rlen) {
            ab_append(ab, rstatus, rlen);
            break;
        } else {
            ab_append(ab, " ", 1);
        }
        len++;
    }
    //ab_append(ab, "\x1b[m", 3);
    ab_append(ab, "\r\n", 2);
}

void editor_draw_message_bar(abuf_t* ab) {
    ab_append(ab, "\x1b[K", 3);
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screen_cols) msglen = E.screen_cols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)
        ab_append(ab, E.statusmsg, msglen);
}

void editor_refresh_screen() {
    editor_scroll();

    abuf_t ab = ABUF_INIT;

    ab_append(&ab, "\x1b[?25l", 6);
    ab_append(&ab, "\x1b[H", 3);

    editor_draw_rows(&ab);
    editor_draw_status_bar(&ab);
    editor_draw_message_bar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.row_off) + 1, (E.rx - E.col_off) + 1);
    ab_append(&ab, buf, strlen(buf));

    ab_append(&ab, "\x1b[?25h", 6);

    write_all(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

void editor_set_status_message(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}

/*** input ***/

char* editor_prompt(char* prompt) {
    size_t bufsize = 128;
    char* buf = malloc(bufsize);

    size_t buflen = 0;
    buf[0] = '\0';

    while (1) {
        editor_set_status_message(prompt, buf);
        editor_refresh_screen();

        int c = editor_read_key();
        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
            if (buflen != 0) buf[--buflen] = '\0';
        } else if (c == '\x1b') {
            editor_set_status_message("");
            free(buf);
            return NULL;
        } else if (c == '\r') {
            if (buflen != 0) {
                editor_set_status_message("");
                return buf;
            }
        } else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }
    }
}

void editor_move_cursor(int key) {
    erow_t* row = NULL;
    if (E.cy < E.num_rows)
        row = &E.rows[E.cy];

    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0) {
                E.cx--;
            } else if (E.cy > 0) {
                E.cy--;
                E.cx = E.rows[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size) {
                E.cx++;
            } else if (row && E.cx == row->size) {
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if (E.cy != 0)
                E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy != E.num_rows)
                E.cy++;
            break;
    }

    row = NULL;
    if (E.cy < E.num_rows)
        row = &E.rows[E.cy];
    int row_len = row ? row->size : 0;
    if (E.cx > row_len)
        E.cx = row_len;
}

void editor_process_keypress() {
    static int quit_times = ED_QUIT_TIMES;

    int c = editor_read_key();

    switch (c) {
        case '\r':
            editor_insert_new_line();
            break;
            
        case CTRL_KEY('q'):
            if (E.dirty && quit_times > 0) {
                editor_set_status_message("WARNING! File has unsaved changes. Press CTRL-Q %d more times to quit.", quit_times);
                quit_times--;
                return;
            }
            write_all(STDOUT_FILENO, "\x1b[2J", 4);
            write_all(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;

        case CTRL_KEY('s'):
            editor_save();
            break;

        case HOME_KEY:
            E.cx = 0;
            break;

        case END_KEY:
            if (E.cy < E.num_rows)
                E.cx = E.rows[E.cy].size;
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY) editor_move_cursor(ARROW_RIGHT);
            editor_del_char();
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (c == PAGE_UP) {
                    E.cy = E.row_off;
                } else if (c == PAGE_DOWN) {
                    E.cy = E.row_off + E.screen_rows - 1;
                    if (E.cy > E.num_rows)
                        E.cy = E.num_rows;
                }

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

        case CTRL_KEY('l'):
        case '\x1b':
            break;

        default:
            editor_insert_char(c);
            break;
    }

    quit_times = ED_QUIT_TIMES;
}

/*** init ***/

void init_editor() {
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.row_off = 0;
    E.col_off = 0;
    E.num_rows = 0;
    E.rows = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;

    if (get_window_size(&E.screen_rows, &E.screen_cols) == -1)
        die("get_window_size");
    E.screen_rows -= 2;
}

int main(int argc, char* argv[]) {
    enable_raw_mode();
    init_editor();
    if (argc >= 2) {
        editor_open(argv[1]);
    }

    editor_set_status_message("HELP: CTRL-S = save | CTRL-Q = quit");

    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
    return 0;
}