#!/bin/bash

set -e

if [[ -z "$ONRAMP_BIN" ]]; then
    echo "Error: ONRAMP_BIN is not set or is empty." >&2
    exit 1
fi

CC="$ONRAMP_BIN/onrampvm $ONRAMP_BIN/../share/onramp/bin/cc.oe"

eval $CC -g -I../lib ../lib/conio.c ../lib/kbd.c main.c -o main.oe
hexdump -v -e '1/4 "%08x\n"' main.oe > main.hex
python3 ../../scripts/disassemble.py main.oe > main.lst
