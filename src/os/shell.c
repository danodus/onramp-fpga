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

// TODO: Move this to a separate executable
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

static bool run_command(const char* cmd) {
    char* buf = strdup(cmd);
    size_t nb_args = 0;
    const char* args[256];

/*
    char* token = strtok(b, " \n");

    while (token) {
        args[args_index++] = token;
        token = strtok(NULL, " \n");
    }
*/

    size_t token_first_char_index = 0;
    size_t i = 0;
    for (;;) {
        if (buf[i] == '\n' || buf[i] == ' ' || buf[i] == '\0') {
            bool end_of_string = (buf[i] == '\0');
            if (i > token_first_char_index) {
                args[nb_args] = &buf[token_first_char_index];
                buf[i] = '\0';
                nb_args++;
                if (nb_args >= sizeof(args) - 1)
                    break;
            }
            token_first_char_index = i + 1;
            if (end_of_string)
                break;
        }
        i++;
    }

    if (nb_args < 1) {
        free(buf);
        return false;
    }

    args[nb_args++] = NULL;

    bool ret = run_program(args[0], args);
    free(buf);

    return ret;
}

static void command_prompt(void) {
    char buf[256];

    for(;;) {
        fputs(">", stdout);
        fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin)) {
            if (!buf[0] || buf[0] == '\n')
                break;
            run_command(buf);
        }
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
            "[p] command prompt\n"
            "\n"
            "[0] clean\n"
            "\n"
            "[1] build \"ld-0-global/ld.oe\"\n"
            "[2] build \"ar-0-cat/ar.oe\"\n"
            "[3] build \"libc-0-oo/libc.oa\"\n"
            "\n"
            "[4] build \"libo-0-oo/libo.oa\"\n"
            "[5] build \"as-0-basic/as.oe\"\n"
            "[6] build \"as-1-compound/as.oe\"\n"
            "\n"
            "[7] build \"cpp-0-strip/cpp.oe\"\n"
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
            case 'P':
            case 'p':
                command_prompt();
                break;                
            case '0':
                remove_files_with_prefix("build/");
                break;
            case '1':
                run_command("hex.oe core/ld/0-global/ld.oe.ohx -o build/ld-0-global/ld.oe");
                break;
            case '2':
                run_command("build/ld-0-global/ld.oe "
                    "core/libc/0-oo/src/start.oo "
                    "core/libc/0-oo/src/ctype.oo "
                    "core/libc/0-oo/src/environ.oo "
                    "core/libc/0-oo/src/errno.oo "
                    "core/libc/0-oo/src/malloc.oo "
                    "core/libc/0-oo/src/malloc_util.oo "
                    "core/libc/0-oo/src/spawn.oo "
                    "core/libc/0-oo/src/stdio.oo "
                    "core/libc/0-oo/src/string.oo "
                    "core/libo/0-oo/src/libo-error.oo "
                    "core/libo/0-oo/src/libo-util.oo "
                    "core/ar/0-cat/ar.oo "
                    "-o build/ar-0-cat/ar.oe"
                );
                break;                
            case '3':
                run_command("build/ar-0-cat/ar.oe "
                    "rc build/libc-0-oo/libc.oa "
                    "core/libc/0-oo/src/start.oo "
                    "core/libc/0-oo/src/ctype.oo "
                    "core/libc/0-oo/src/environ.oo "
                    "core/libc/0-oo/src/errno.oo "
                    "core/libc/0-oo/src/malloc.oo "
                    "core/libc/0-oo/src/malloc_util.oo "
                    "core/libc/0-oo/src/spawn.oo "
                    "core/libc/0-oo/src/stdio.oo "
                    "core/libc/0-oo/src/string.oo"
                );
                break;
            case '4':
                run_command("build/ar-0-cat/ar.oe "
                    "rc build/libo-0-oo/libo.oa "
                    "core/libo/0-oo/src/libo-error.oo "
                    "core/libo/0-oo/src/libo-util.oo "
                );
                break;
            case '5':
                run_command("build/ld-0-global/ld.oe "
                    "-o build/as-0-basic/as.oe "
                    "build/libc-0-oo/libc.oa "
                    "build/libo-0-oo/libo.oa "
                    "core/as/0-basic/as.oo "
                );
                break;
            case '6':
                run_command("build/as-0-basic/as.oe "
                    "core/as/1-compound/src/emit.os "
                    "-o build/as-1-compound/emit.oo"
                );
                run_command("build/as-0-basic/as.oe "
                    "core/as/1-compound/src/main.os "
                    "-o build/as-1-compound/main.oo"
                );
                run_command("build/as-0-basic/as.oe "
                    "core/as/1-compound/src/op_arithmetic.os "
                    "-o build/as-1-compound/op_arithmetic.oo"
                );
                run_command("build/as-0-basic/as.oe "
                    "core/as/1-compound/src/op_control.os "
                    "-o build/as-1-compound/op_control.oo"
                );
                run_command("build/as-0-basic/as.oe "
                    "core/as/1-compound/src/op_logic.os "
                    "-o build/as-1-compound/op_logic.oo"
                );
                run_command("build/as-0-basic/as.oe "
                    "core/as/1-compound/src/op_memory.os "
                    "-o build/as-1-compound/op_memory.oo"
                );
                run_command("build/as-0-basic/as.oe "
                    "core/as/1-compound/src/opcodes.os "
                    "-o build/as-1-compound/opcodes.oo"
                );
                run_command("build/as-0-basic/as.oe "
                    "core/as/1-compound/src/parse.os "
                    "-o build/as-1-compound/parse.oo"
                );
                run_command("build/ld-0-global/ld.oe "
                    "build/libc-0-oo/libc.oa "
                    "build/libo-0-oo/libo.oa "
                    "build/as-1-compound/emit.oo "
                    "build/as-1-compound/main.oo "
                    "build/as-1-compound/op_arithmetic.oo "
                    "build/as-1-compound/op_control.oo "
                    "build/as-1-compound/op_logic.oo "
                    "build/as-1-compound/op_memory.oo "
                    "build/as-1-compound/opcodes.oo "
                    "build/as-1-compound/parse.oo "
                    "-o build/as-1-compound/as.oe"
                );
                break;
            case '7':
                run_command("build/as-1-compound/as.oe "
                    "core/cpp/0-strip/cpp.os "
                    "-o build/cpp-0-strip/cpp.oo"
                );
                run_command("build/ld-0-global/ld.oe "
                    "build/libc-0-oo/libc.oa "
                    "build/libo-0-oo/libo.oa "
                    "build/cpp-0-strip/cpp.oo "
                    "-o build/cpp-0-strip/cpp.oe"
                );
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