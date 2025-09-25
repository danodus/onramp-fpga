build/intermediate/cpp-1-omc/cpp.oe main.c -o main.i
build/intermediate/cci-0-omc/cci.oe main.i -o main.os
build/intermediate/as-1-compound/as.oe main.os -o main.oo
build/intermediate/ld-1-omc/ld.oe build/intermediate/libc-1-omc/libc.oa main.oo -o main.oe
