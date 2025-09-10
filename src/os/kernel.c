// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <spawn.h>
#include <__onramp/__pit.h>

#define CFG         0x20000000
#define LED         0x20001000

extern unsigned* __process_info_table;

int __sys_dopen(const char* path);
int __sys_dread(int handle, char out_buffer[256]);
int __sys_stat(const char* path, unsigned output[4]);

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
    if (__sys_dopen(".") != 0) {
        printf("sys_dread failed\n");
        return;
    }
/*
    char path[256];
    
    for (;;) {
        if (__sys_dread(0, path) != 0) {
            printf("sys_dread failed\n");
            return;
        }
        if (path[0] == '\0')
            break;
        unsigned int file_size = get_file_size(path);
        printf("%s\t%d\n", path, file_size);
    }
*/        
}

bool run_program(const char* filename) {
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

    printf("Execute %s...\n", filename);

    // Allocate a process information table for the child as a copy of ours
    unsigned int* parent_pit = __process_info_table;
    unsigned int* child_pit = __memdup(parent_pit, sizeof(int) * 12);

    const char *args[] = {
        filename,
        "hello.ohx",
        "-o",
        "hello.bin",
        NULL
    };

    // Setup the child pit
    *(child_pit + __ONRAMP_PIT_ARGS) = (int)args;

    // Run it
    int ret = __onramp_spawn_pit(program, program_size, child_pit, filename);
    
    free(child_pit);
    free(program);
    return true;
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
            "[r] run \"hex.oe hello.ohx -o hello.bin\"\n"
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
            case 'R':
            case 'r':
                if (!run_program("hex.oe"))
                    printf("Unable to run the program\n");
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