// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

// Ref.: Line editor from Queso Fuego (https://www.youtube.com/watch?v=t4gXccB2M48&list=PLT7NbkyNWaqZ6zepmv7c6DsyoL0gANPqC)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include <conio.h>

char* lines[2000], line[BUFSIZ];
int a1, a2, a3, last_line, curr_line;
char cmd;
bool have_a1, have_a2, have_a3; // Did we parse out addresses from the last command line read in?
char* filename;
char* cmd_parms;

char* str_dup(char* s) {
    char* new = malloc(strlen(s) + 1);
    if (!new) return NULL;
    strcpy(new, s);
    return new;
}

// (.)a: Append new lines to buffer after given address
bool cmd_append() {
    if (!have_a1) { a2 = a1 = curr_line; have_a1 = true; }
    while (read_line(line, sizeof(line))) {
        if (!strcmp(line, ".\n")) break;        // Done inputting lines with single '.'
        a1++;   // Will append after given address 1
        // Move curret buffer lines to make rooom for new line
        memmove(&lines[a1+1], &lines[a1], (last_line - a1 + 1) * sizeof(*lines));
        lines[a1] = str_dup(line);
        last_line++;
        curr_line = a1;  // '.' = last line added to the buffer
    }
    return true;
}

// (.)i: Same as cmd_append, but add _at_ the given address, _not_ after it
bool cmd_insert() {
    // Function same as cmd_append, but add _at_ the given address a1, _not_ after it
    if (a1 > 0) a1--;
    return cmd_append();
}

// (.,.)d: Delete line range from buffer
bool cmd_delete() {
    if (!have_a1) { a2 = a1 = curr_line; have_a1 = true; }
    // Free line memory 1st to not have leaks
    for (int i = a1; i <= a2; i++)
        free(lines[i]);
    // Overwrite deleted lines that have noew freed up
    memmove(&lines[a1], &lines[a2+1], (last_line - a2) * sizeof(*lines));
    last_line -= a2 - a1 + 1;   // Removed lines from buffer
    curr_line = (a1 < last_line) ? a1 : last_line;  // '.' = last line deleted, or new last line in buffer
    return true;
}

// (.,.)c: Change line range from buffer
bool cmd_change() {
    // Delete then add new lines in their place, thereby "changing" those lines
    return cmd_delete() && cmd_insert();
}

// (.,.)n: Print lines with their line numbers
bool cmd_print_with_line_number() {
    if (!have_a1) { a2 = a1 = curr_line; have_a1 = true; }
    for (int i = a1; i <= a2; i++)
        printf("%d\t%s", i, lines[i]);
    curr_line = a2; // '.' last line printed
    return true;
}

// (.,.)n: Print lines plainly
bool cmd_print() {
    if (!have_a1) { a2 = a1 = curr_line; have_a1 = true; }
    for (int i = a1; i <= a2; i++)
        printf("%s", lines[i]);
    curr_line = a2; // '.' last line printed
    return true;
}

// (1,$)w: Write lines to current filename
bool cmd_write() {
    if (!have_a1) { a1 = 1; a2 = last_line; have_a1 = true; }
    FILE* fp = fopen(filename, "w");
    if (!fp) return false;
    size_t bytes_written = 0;
    for (int i = a1; i <= a2; i++) {
        fputs(lines[i], fp);
        bytes_written += strlen(lines[i]);
    }
    fclose(fp);
    // TODO: Check future "-s" command line flag
    printf("%d\n", (int)bytes_written);
    return true;
}

bool cmd_print_address() {
    // Print line number of given address
    printf("%d\n", a1);
    return true;
}

// Get next line range and command to run
bool next_address(char** ln, int* addr) {
    *addr = -1;
    char* s = *ln;
    for (bool first = true; ; first = false) {
        while (isblank(*s)) s++;
        char c = *s;
        switch (c) {
            case '$': case '.':
                if (!first) return false;   // Needs to be first component in address
                *addr = (c == '$') ? last_line : curr_line;   // '$' or '.'
                s++;
            break;

            case '/': case '?':
                if (!first) return false;
                // TODO:
                s++;
            break;

            case '+': case '-':
                if (first) {
                    if (first) *addr = curr_line;   // else will offset from current line (addr)
                    // Set addr + or - if single +/-, else +/- after the sign
                    s++;
                    if (!isdigit(*s)) *addr += (c == '+') ? 1 : -1;
                    else {
                        *addr += (c == '+') ? atoi(s) : -atoi(s);
                        while (isdigit(*s)) s++;
                    }
                }
            break;

            case '\'':
                // TODO
            break;

            case '0': case '1': case '2': case '3':  case '4':
            case '5': case '6': case '7': case '8':  case '9':
                // Number, either set addr to this number, or offset from current address
                if (first) *addr = atoi(s);
                else *addr += atoi(s);
                while (isdigit(*s)) s++;
            break;

            default:
            goto done;  // No more address components, leave loop
        }
    }
    done:
    if (*addr < -1 || *addr > last_line)
        return false;
    *ln = s;    // Update line to skip consumed characters
    return true;
}

bool next_command() {
    // Print prompt by default
    printf("*"), fflush(stdout);
    // Read in next line addresses and command to run
    if (!read_line(line, sizeof(line), stdin)) return false;
    a1 = a2 = a3 = 0;
    have_a1 = have_a2 = have_a3 = false;
    cmd = '\0';
    char* s = line; // Command line to parse into addresses, etc.
    if (!next_address(&s, &a1)) return false;
    have_a1 = (a1 > -1);
    while (*s == ',' || *s == ';') {
        // Have multiple addresses, also parse them out or set defaults as needed for ','/';'
        if (have_a2) a1 = a2;
        if (!have_a1) a1 = (*s == ',') ? 1 : curr_line; // Default ',' = 1,$ L else ';' = .,$
        if (*s == ';') curr_line = a1;
        s++;    // Skip over ,/;

        // Get next address 2
        if (!next_address(&s, &a2)) return false;
        have_a2 = (a2 > -1);
        if (!have_a2) a2 = (have_a1) ? a1 : last_line;
        have_a2 = have_a1 = true;   // Get both addresses now
    }
    if (!have_a2) a2 = a1;  // Default to 1st address if needed, e.g. for (.,.) command default lines
    // Get command to run
    cmd = *s++;
    if (!strchr("acidnpw=q", cmd))
        return false;
    while (isblank(*s)) s++;
    cmd_parms = s;
    return true;
}

int main(int argc, char *argv[]) {

    if (argc < 2)
        return EXIT_FAILURE;
    // Read in input file to line buffer
    filename = argv[1];
    FILE* fp = fopen(filename, "r");
    if (!fp) return EXIT_FAILURE;
    size_t bytes_read = 0;
    while (fgets(line, sizeof(line), fp)) {
        lines[++last_line] = str_dup(line);
        bytes_read += strlen(line);
    }
    fclose(fp);
    printf("%d\n", (int)bytes_read);
    curr_line = last_line;   // Set '.' initially to last line read in from file

    // Input loop to read next command to run on lines
    bool running = true;
    while (running) {
        if (!next_command()) {
            puts("?");
        }
        // Run command on lines
        switch (cmd) {
            case 'a':   // Append lines to the buffer
            cmd_append();
            break;

            case 'c':   // Change lines to the buffer
            cmd_change();
            break;

            case 'i':   // Insert lines to the buffer
            cmd_insert();
            break;

            case 'd':   // Delete lines from the buffer
            cmd_delete();
            break;

            case 'n':   // Print lines with line number
            cmd_print_with_line_number();
            break;

            case 'p':   // Print lines without line number
            cmd_print();
            break;

            case 'w':   // Write lines to file
            cmd_write();
            break;

            case '=':   // Print last line
            cmd_print_address();
            break;

            case 'q':   // Quit program
            running = false;
            break;
        }
    }

    // Free remaining line memory to be nice
    for (int i = 1; i <= last_line; i++)
        free(lines[i]);

    return EXIT_SUCCESS;
}