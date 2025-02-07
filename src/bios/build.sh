onramphex bios.ohx -o bios.oe
hexdump -v -e '1/4 "%08x\n"' bios.oe > bios.hex