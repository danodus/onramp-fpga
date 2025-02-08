onrampcc bios.c -S -o bios.os
onrampcc -nostdlib start.os bios.os -o bios.oe
hexdump -v -e '1/4 "%08x\n"' bios.oe > bios.hex
