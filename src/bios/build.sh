if [[ -z "$ONRAMP_BIN" ]]; then
    echo "Error: ONRAMP_BIN is not set or is empty." >&2
    return 1
fi

CC="$ONRAMP_BIN/onrampvm $ONRAMP_BIN/../share/onramp/bin/cc.oe"

eval $CC -nostdinc io.c -S -emit-ir -o io.os
eval $CC -nostdinc bios.c -S -emit-ir -o bios.os
eval $CC -nostdlib start.os io.os bios.os -o bios.oe
hexdump -v -e '1/4 "%08x\n"' bios.oe > bios.hex
