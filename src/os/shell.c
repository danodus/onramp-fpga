// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

#include <spawn.h>
#include <__onramp/__pit.h>

#include <config.h>
#include <conio.h>
#include <crc.h>

#define CFG         0x20000000
#define LED         0x20001000

extern unsigned* __process_info_table;

int __sys_dopen(const char* path);
int __sys_dread(int handle, char out_buffer[256]);
int __sys_stat(const char* path, unsigned output[4]);
int __sys_unlink(const char* path);
int __sys_fclose(int file_handle);
int __sys_rename(const char* from, const char* to);

bool file_open_table[MAX_OPEN_FILES] = {false};

int last_ret = 0;

FILE* fopen_t(const char* restrict filename, const char* restrict mode) {
    FILE* file = fopen(filename, mode);
    if (file != NULL)
        file_open_table[fileno(file) - 3] = true;
    return file;
}

int fclose_t(FILE* file) {
    file_open_table[fileno(file) - 3] = false;
    return fclose(file);
}

int is_hardware(void) {
    return *(int *)(CFG) & 1;
}

void set_led(int value) {
    if (is_hardware()) {
         *(int *)(LED) = value;
    }
}

void exit_handler(void) {
    set_led(0x55);
}

unsigned int get_file_size(const char* filename) {
    unsigned int stat[4];
    if (__sys_stat(filename, stat) != 0) {
        printf("sys_stat failed\n");
        return 0;
    };
    return stat[2];
}

void list_files(void) {
    if (__sys_dopen(NULL) != 0) {
        printf("sys_dread failed\n");
        return;
    }

    char path[256];
    
    for (;;) {
        if (__sys_dread(0, path) != 0) {
            printf("sys_dread failed\n");
            return;
        }
        if (path[0] == '\0')
            break;
        unsigned int file_size = get_file_size(path);
        printf("%s\t\t%d\n", path, file_size);
    }
}

bool remove_file_with_prefix(const char* prefix) {
    if (__sys_dopen(NULL) != 0) {
        printf("sys_dread failed\n");
        return false;
    }

    char path[256];
    
    bool removed = false;

    size_t prefix_len = strlen(prefix);
    for (;;) {
        if (__sys_dread(0, path) != 0) {
            printf("sys_dread failed\n");
            return false;
        }
        if (path[0] == '\0')
            break;
        if (strncmp(prefix, path, prefix_len) == 0) {
            printf("Removing %s\n", path);
            __sys_unlink(path);
            removed = true;
            break;
        }
    }

    return removed;
}

void remove_files_with_prefix(const char* prefix) {
    while (remove_file_with_prefix(prefix));
}

bool run_program(const char* filename, const char *args[]) {
    unsigned int program_size = get_file_size(filename);
    
    if (program_size == 0) {
        printf("%s is not found or empty\n", filename);
        return false;
    }

    FILE* f = fopen_t(filename, "rb");
    if (f == NULL) {
        printf("Program not found\n");
        return false;
    }

    char* program = malloc(program_size);
    if (program == NULL) {
        printf("Out of memory\n");
        fclose_t(f);
        return false;
    }


    if (fread(program, 1, program_size, f) != program_size) {
        printf("Unable to read the program\n");
        fclose_t(f);
        free(program);
        return false;
    };

    fclose_t(f);

    // Check if this is an Onramp program
    if (strncmp(program, "~Onr~amp~   ", 12) != 0) {
        printf("Not an Onramp program\n");
        free(program);
        return false;
    }

    printf("Executing \"%s", filename);
    if (args[0]) {
        size_t i = 1;
        while (args[i])
            printf(" %s", args[i++]);
    }
    printf("\"...\n");

    // Allocate a process information table for the child as a copy of ours
    unsigned int* parent_pit = __process_info_table;
    unsigned int* child_pit = __memdup(parent_pit, sizeof(int) * 12);

    // Setup the child pit
    *(child_pit + __ONRAMP_PIT_ARGS) = (int)args;

    // Run it
    last_ret = __onramp_spawn_pit(program, program_size, child_pit, filename);
    
    // Clean up by forcefully close all remaining open files
    for (int i = 0; i < MAX_OPEN_FILES; ++i) {
        if (!file_open_table[i])
            __sys_fclose(i + 3);
    }

    free(child_pit);
    free(program);
    return (last_ret == 0) ? true : false;
}

void cat(const char* filename) {
    FILE* f;
    char buf[256];
    char* ss;
    f = fopen_t(filename, "rb");
    if (f != NULL) {
        size_t n;
        do {
            n = fread(buf, 1, sizeof(buf), f);
            fwrite(buf, 1, n, stdout);
        } while (n > 0);

        fclose_t(f);
    } else {
        printf("file not found\n");
    }
}

void xxd(const char* filename) {
    FILE* f;
    uint8_t buf[256];
    char* ss;
    f = fopen_t(filename, "rb");
    if (f != NULL) {
        size_t n;
        do {
            n = fread(buf, 1, sizeof(buf), f);
            for (int i = 0; i < n; ++i) {
                if (i % 32 == 0)
                    printf("\n");
                printf("%02x ", buf[i]);
            }
        } while (n > 0);
        printf("\n");

        fclose_t(f);
    } else {
        printf("file not found\n");
    }
}

static bool copy_file(const char* src_filename, const char* dst_filename) {
    FILE* src_f;
    FILE* dst_f;
    src_f = fopen_t(src_filename, "rb");
    if (src_f == NULL) {
        printf("Unable to open %s\n", src_filename);
        return false;
    }
    dst_f = fopen_t(dst_filename, "wb");
    if (dst_f == NULL) {
        printf("Unable to open %s\n", dst_filename);
        fclose_t(src_f);
        return false;
    }

    size_t n;
    uint8_t buf[256];
    do {
        n = fread(buf, 1, sizeof(buf), src_f);
        if (fwrite(buf, 1, n, dst_f) != n) {
            printf("Unable to write\n");
            fclose_t(dst_f);
            fclose_t(src_f);
            return false;
        }
    } while (n > 0);

    fclose_t(dst_f);
    fclose_t(src_f);

    return true;
}

static bool move_file(const char* src_filename, const char* dst_filename) {
    if (__sys_rename(src_filename, dst_filename)) {
        printf("File not found\n");
        return false;
    }
    return true;
}

static bool touch(const char* filename) {
    FILE *f = fopen_t(filename, "wb");
    if (f == NULL) {
        printf("Unable to open %s\n", filename);
        return false;
    }
    fclose(f);
    return true;
}

static bool crc(const char* filename) {
    // Read in input file to line buffer
    FILE* f = fopen(filename, "r");
    if (!f) return EXIT_FAILURE;

    uint32_t checksum_accum = 0xffffffffu;
    for (;;) {
        uint8_t v = fgetc(f);
        if (feof(f))
            break;
        checksum_accum = crc_checksum_byte(CRC_POLY_CRC32, checksum_accum, v);
    }

    fclose(f);

    printf("CRC32: %x\n", checksum_accum);

}

static bool run_command(const char *args[], bool show_time);

// Thanks to Haelwenn (lanodan) Monnier for this script parser
static bool parse_script(FILE* in)
{
    bool c_was_space = false;
    size_t args_buf_i = 0;
    size_t args_i = 0;
    char args_buf[4096] = "";
    const char* args[256] = {&args_buf[0]};

    for (;;) {
        int c = fgetc(in);

    got_c:
        if (c == '\\') {
            c = fgetc(in);

            if(c == '\n') continue;

            fputs("sh-oe: error: Invalid escaped char: ", stdout);
            fputc(c, stdout);
            fputs("\n", stdout);
            fflush(stdout);
            return false;
        }

        if (c == EOF || c == '\n' || c == '\r' || c == '#') {
            if (args_buf_i == 0) {
                // done
                if(c == EOF) return true;

                // empty line
                if(c != '#') continue;

                for (;;) {
                    c = fgetc(in);
                    if(c == '\n' || c == '\r' || c == EOF) break;
                }

                goto got_c;
            }

            if (args_i > 0 || args_buf_i > 0) {
                args_buf[args_buf_i++] = '\0';
                args[++args_i] = NULL;

                bool ret = run_command(args, false);
                if (!ret) return false;
            }

            args_buf_i = 0;
            args_i = 0;
            c_was_space = false;
        } else if (isspace(c)) {
            if (args_buf_i == 0) continue;
            if (c_was_space) continue;

            if (args_i > 255) {
                fputs("sh-oe: error: too many arguments\n", stdout);
                return false;
            }

            c_was_space = true;
        } else {
            if (c_was_space) {
                args_buf[args_buf_i++] = '\0';
                args[++args_i] = &args_buf[args_buf_i];
                c_was_space = false;
            }

            args_buf[args_buf_i++] = c;
        }
    }

    return true;
}

static bool run_script(const char* filename) {
    FILE* f = fopen_t(filename, "rb");
    bool ret = false;
    if (f != NULL) {
        ret = parse_script(f);
        fclose_t(f);
    } else {
        printf("File not found\n");
    }
    return ret;
}

static void print_help(void) {
    printf("The built-in commands are: help echo onrampvm exit time ls cat xxd rm rmall cp mv touch ret crc\n");
}

static bool run_command(const char *args[], bool show_time) {

    time_t start_time, end_time;
    double elapsed_seconds;

    start_time = clock();

    bool ret = true;
    const char* ext = strrchr(args[0], '.');
    if (ext) {
        ext++;
        if (strcmp(ext, "oe") == 0) {
            ret = run_program(args[0], args);
        } else if (strcmp(ext, "sh") == 0) {
            ret = run_script(args[0]);
        }
    } else {
        // internal commands
        if (strcmp(args[0], "set") == 0 || strcmp(args[0], "mkdir") == 0) {
            // discarded            
        } else if (strcmp(args[0], "echo") == 0) {
            for (int i = 1; args[i]; i++)
                printf("%s ", args[i]);
            printf("\n");
        } else if (strcmp(args[0], "onrampvm") == 0) {
            ret = run_command(&args[1], false);
        } else if (strcmp(args[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(args[0], "time") == 0) {
            ret = run_command(&args[1], true);
        } else if (strcmp(args[0], "ls") == 0) {
            list_files();
        } else if (strcmp(args[0], "cat") == 0) {
            if (args[1])
                cat(args[1]);
        } else if (strcmp(args[0], "xxd") == 0) {
            if (args[1])
                xxd(args[1]);
        } else if (strcmp(args[0], "rm") == 0) {
            if (args[1]) {
                // TODO: Use remove() instead when available
                // remove(args[1]);
                __sys_unlink(args[1]);
            }
        } else if (strcmp(args[0], "rmall") == 0) {
            if (args[1])
                remove_files_with_prefix(args[1]);
        } else if (strcmp(args[0], "cp") == 0) {
            if (args[1] && args[2])
                copy_file(args[1], args[2]);
        } else if (strcmp(args[0], "mv") == 0) {
            if (args[1] && args[2])
                move_file(args[1], args[2]);
        } else if (strcmp(args[0], "touch") == 0) {
            if (args[1])
                touch(args[1]);
        } else if (strcmp(args[0], "ret") == 0) {
            printf("Last return value: %d\n", last_ret);
        } else if (strcmp(args[0], "crc") == 0) {
            if (args[1])
                crc(args[1]);
        } else if (strcmp(args[0], "help") == 0) {
            print_help();
        } else printf("Unknown command\n");
    }

    end_time = clock();

    if (show_time)
        printf("Time elapsed: %d seconds\n", (end_time - start_time) / CLOCKS_PER_SEC);

    return ret;
}

static bool run_string_command(const char* cmd) {
    char* buf = strdup(cmd);
    size_t nb_args = 0;
    const char* args[256];

    char* token = strtok(buf, " \n");

    while (token) {
        if (nb_args == 255)
            break;
        args[nb_args++] = token;
        token = strtok(NULL, " \n");
    }

    if (nb_args < 1) {
        free(buf);
        return false;
    }

    args[nb_args++] = NULL;

    bool ret = run_command(args, false);

    free(buf);

    return ret;
}

static void command_prompt(void) {
    char buf[1024];

    for(;;) {
        fputs(">", stdout);
        fflush(stdout);
        if (read_line(buf, sizeof(buf) - 1)) {
            if (buf[0] && buf[0] != '\n')
                run_string_command(buf);
        }
    }
}

int main(int argc, char *argv[]) {

    atexit(exit_handler);
    printf("Onramp-FPGA Shell\n");

    command_prompt();

    printf("Bye!\n");

    return 0;
}