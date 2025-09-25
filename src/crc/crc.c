// Copyright (c) 2025 Daniel Cliche
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include <crc.h>

int main(int argc, char *argv[]) {

    if (argc < 2)
        return EXIT_FAILURE;
        
    // Read in input file to line buffer
    char* filename = argv[1];
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

    return EXIT_SUCCESS;
}