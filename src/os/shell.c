// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <spawn.h>
#include <__onramp/__pit.h>

#define CFG         0x20000000
#define LED         0x20001000

extern unsigned* __process_info_table;

int __sys_dopen(const char* path);
int __sys_dread(int handle, char out_buffer[256]);
int __sys_stat(const char* path, unsigned output[4]);
int __sys_unlink(const char* path);

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

bool run_program(const char* filename, const char *args[]) {
    unsigned int program_size = get_file_size(filename);
    
    if (program_size == 0) {
        printf("%s is not found or empty\n", filename);
        return false;
    }

    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        printf("Program not found\n");
        return false;
    }

    char* program = malloc(program_size);
    if (program == NULL) {
        printf("Out of memory\n");
        fclose(f);
        return false;
    }


    if (fread(program, 1, program_size, f) != program_size) {
        printf("Unable to read the program\n");
        fclose(f);
        free(program);
        return false;
    };

    fclose(f);

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
    int ret = __onramp_spawn_pit(program, program_size, child_pit, filename);
    
    free(child_pit);
    free(program);
    return true;
}

void cat(const char* filename) {
    FILE* f;
    char buf[256];
    char* ss;
    f = fopen(filename, "rb");
    if (f != NULL) {
        size_t n;
        do {
            n = fread(buf, 1, sizeof(buf), f);
            fwrite(buf, 1, n, stdout);
        } while (n > 0);

        fclose(f);
    } else {
        printf("file not found\n");
    }
}

void xxd(const char* filename) {
    FILE* f;
    uint8_t buf[256];
    char* ss;
    f = fopen(filename, "rb");
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

        fclose(f);
    } else {
        printf("file not found\n");
    }
}

int main(int argc, char *argv[]) {

    atexit(exit_handler);
    printf("Onramp-FPGA OS\n");

    printf("Number of arguments: %d\n", argc);
    for (int i = 0; i < argc; ++i) {
        printf("  argv[%d]=%s\n", i, argv[i]);
    }

    bool quit = false;
    while (!quit) {
        printf(
            "\n"
            "[l] list files\n"
            "\n"
            "[x] xxd \"hex.oe\"\n"
            "[r] run \"hex.oe hello.ohx -o hello.txt\"\n"
            "[c] cat \"hello.txt\"\n"
            "\n"
            "[0] clean\n"
            "\n"
            "[1] build \"ld-0-global/ld.oe\"\n"
            "[2] build \"ar-0-cat/ar.oe\"\n"
            "[3] build \"libc-0-oo/libc.oa\"\n"
            "\n"
            "[4] build \"libo-0-oo/libo.oa\"\n"
            "[5] build \"as-0-basic/as.oe\"\n"
            "[6] build partial \"as-1-compound/as.oe\" (only \"build/as-1-compound/emit.oo\")\n"
            "[7] cat \"build/as-1-compound/emit.oo\"\n"
            "\n"
            "[q] quit\n"
            "Make a selection...\n"
        );
        int c = getchar();
        printf("\n");
        switch (c) {
            case 'L':
            case 'l':
                list_files();
                break;
            case 'X':
            case 'x':
                xxd("hex.oe");
                break;                
            case 'R':
            case 'r':
                {
                    const char *args[] = {
                        "hex.oe",
                        "hello.ohx",
                        "-o",
                        "hello.txt",
                        NULL
                    }; 
                    run_program("hex.oe", args);
                }
                break;
            case 'C':
            case 'c':
                cat("hello.txt");
                break;
            case '0':
                if (__sys_unlink("build/ld-0-global/ld.oe")     ||
                    __sys_unlink("build/ar-0-cat/ar.oe")        ||
                    __sys_unlink("build/libc-0-oo/libc.oa")     ||
                    __sys_unlink("build/libo-0-oo/libo.oa")     ||
                    __sys_unlink("build/as-0-basic/as.oe")      ||
                    __sys_unlink("build/as-1-compound/emit.oo") // ||
                    //__sys_unlink("build/as-1-compound/as.oe")
                    ) {
                        //printf("One or more files could not be removed\n");
                    }
                break;
            case '1':
                {
                    const char *args[] = {
                        "hex.oe",
                        "core/ld/0-global/ld.oe.ohx",
                        "-o",
                        "build/ld-0-global/ld.oe",
                        NULL
                    };
                    run_program("hex.oe", args);
                }
                break;
            case '2':
                {
                    const char *args[] = {
                        "ld.oe",
                        "core/libc/0-oo/src/start.oo",
                        "core/libc/0-oo/src/ctype.oo",
                        "core/libc/0-oo/src/environ.oo",
                        "core/libc/0-oo/src/errno.oo",
                        "core/libc/0-oo/src/malloc.oo",
                        "core/libc/0-oo/src/malloc_util.oo",
                        "core/libc/0-oo/src/spawn.oo",
                        "core/libc/0-oo/src/stdio.oo",
                        "core/libc/0-oo/src/string.oo",
                        "core/libo/0-oo/src/libo-error.oo",
                        "core/libo/0-oo/src/libo-util.oo",
                        "core/ar/0-cat/ar.oo",
                        "-o",
                        "build/ar-0-cat/ar.oe",
                        NULL
                    };
                    run_program("build/ld-0-global/ld.oe", args);
                }
                break;                
            case '3':
                {
                    const char *args[] = {
                        "ar.oe",
                        "rc",
                        "build/libc-0-oo/libc.oa",
                        "core/libc/0-oo/src/start.oo",
                        "core/libc/0-oo/src/ctype.oo",
                        "core/libc/0-oo/src/environ.oo",
                        "core/libc/0-oo/src/errno.oo",
                        "core/libc/0-oo/src/malloc.oo",
                        "core/libc/0-oo/src/malloc_util.oo",
                        "core/libc/0-oo/src/spawn.oo",
                        "core/libc/0-oo/src/stdio.oo",
                        "core/libc/0-oo/src/string.oo",
                        NULL
                    };
                    run_program("build/ar-0-cat/ar.oe", args);
                }
                break;
                case '4':
                {
                    const char *args[] = {
                        "ar.oe",
                        "rc",
                        "build/libo-0-oo/libo.oa",
                        "core/libo/0-oo/src/libo-error.oo",
                        "core/libo/0-oo/src/libo-util.oo",
                        NULL
                    };
                    run_program("build/ar-0-cat/ar.oe", args);
                }
                break;
                case '5':
                {
                    const char *args[] = {
                        "ld.oe",
                        "-o",
                        "build/as-0-basic/as.oe",
                        "build/libc-0-oo/libc.oa",
                        "build/libo-0-oo/libo.oa",
                        "core/as/0-basic/as.oo",
                        NULL
                    };
                    run_program("build/ld-0-global/ld.oe", args);
                }
                break;
                case '6':
                // TODO: create a run_command() to simplify 
                {
                    const char *args[] = {
                        "as.oe",
                        "core/as/1-compound/src/emit.os",
                        "-o",
                        "build/as-1-compound/emit.oo",
                        NULL
                    };
                    run_program("build/as-0-basic/as.oe", args);
                }
                // TODO: Remaining
                /*
                {
                    const char *args[] = {
                        "as.oe",
                        "core/as/1-compound/src/main.os",
                        "-o",
                        "build/as-1-compound/main.oo",
                        NULL
                    };
                    run_program("build/as-0-basic/as.oe", args);
                }
                {
                    const char *args[] = {
                        "as.oe",
                        "core/as/1-compound/src/op_arithmetic.os",
                        "-o",
                        "build/as-1-compound/op_arithmetic.oo",
                        NULL
                    };
                    run_program("build/as-0-basic/as.oe", args);
                }
                {
                    const char *args[] = {
                        "as.oe",
                        "core/as/1-compound/src/op_control.os",
                        "-o",
                        "build/as-1-compound/op_control.oo",
                        NULL
                    };
                    run_program("build/as-0-basic/as.oe", args);
                }
                {
                    const char *args[] = {
                        "as.oe",
                        "core/as/1-compound/src/op_logic.os",
                        "-o",
                        "build/as-1-compound/op_logic.oo",
                        NULL
                    };
                    run_program("build/as-0-basic/as.oe", args);
                }
                {
                    const char *args[] = {
                        "as.oe",
                        "core/as/1-compound/src/op_memory.os",
                        "-o",
                        "build/as-1-compound/op_memory.oo",
                        NULL
                    };
                    run_program("build/as-0-basic/as.oe", args);
                }
                {
                    const char *args[] = {
                        "as.oe",
                        "core/as/1-compound/src/opcodes.os",
                        "-o",
                        "build/as-1-compound/opcodes.oo",
                        NULL
                    };
                    run_program("build/as-0-basic/as.oe", args);
                }
                {
                    const char *args[] = {
                        "as.oe",
                        "core/as/1-compound/src/parse.os",
                        "-o",
                        "build/as-1-compound/parse.oo",
                        NULL
                    };
                    run_program("build/as-0-basic/as.oe", args);
                }
                {
                    const char *args[] = {
                        "ld.oe",
                        "build/libc-0-oo/libc.oa",
                        "build/libo-0-oo/libo.oa",
                        "build/as-1-compound/emit.oo",
                        "build/as-1-compound/main.oo",
                        "build/as-1-compound/op_arithmetic.oo",
                        "build/as-1-compound/op_control.oo",
                        "build/as-1-compound/op_logic.oo",
                        "build/as-1-compound/op_memory.oo",
                        "build/as-1-compound/opcodes.oo",
                        "build/as-1-compound/parse.oo",
                        "-o",
                        "build/as-1-compound/as.oe",
                        NULL
                    };
                    run_program("build/ld-0-global/ld.oe", args);
                }
                */
                break;
            case '7':
                cat("build/as-1-compound/emit.oo");
                break;
            case 'Q':
            case 'q':
                quit = true;
                break;
            default:
                printf("Invalid selection\n");
        }
    }

    printf("Bye!\n");

    return 0;
}