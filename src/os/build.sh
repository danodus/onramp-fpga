if [[ -z "$ONRAMP_BIN" ]]; then
    echo "Error: ONRAMP_BIN is not set or is empty." >&2
    return 1
fi

CC="$ONRAMP_BIN/onrampvm $ONRAMP_BIN/../share/onramp/bin/cc.oe"

eval $CC -nostdinc -g kernel.c -S -emit-ir -o kernel.os
eval $CC -nostdlib -g start.os kernel.os -o kernel.oe
hexdump -v -e '1/4 "%08x\n"' kernel.oe > kernel.hex
python3 ../../scripts/disassemble.py kernel.oe > kernel.lst
