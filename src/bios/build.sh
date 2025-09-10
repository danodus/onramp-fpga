#!/bin/bash

set -e

if [[ -z "$ONRAMP_BIN" ]]; then
    echo "Error: ONRAMP_BIN is not set or is empty." >&2
    exit 1
fi

CC="$ONRAMP_BIN/onrampvm $ONRAMP_BIN/../share/onramp/bin/cc.oe"

eval $CC -nostdlib -nostdinc -g -DDEBUG -I ../lib start.os common.c io.c sdc.c bio.c fs.c sys.c bios.c -o bios.oe
hexdump -v -e '1/4 "%08x\n"' bios.oe > bios.hex
python3 ../../scripts/disassemble.py bios.oe > bios.lst
