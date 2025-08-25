#!/bin/sh

set -e

if [[ -z "$ONRAMP_BIN" ]]; then
    echo "Error: ONRAMP_BIN is not set or is empty." >&2
    exit 1
fi

CC="$ONRAMP_BIN/onrampvm $ONRAMP_BIN/../share/onramp/bin/cc.oe"

eval $CC -nostdlib -g start.os syscalls.os system.c kernel.c -o kernel.oe
hexdump -v -e '1/4 "%08x\n"' kernel.oe > kernel.hex
python3 ../../scripts/disassemble.py kernel.oe > kernel.lst
