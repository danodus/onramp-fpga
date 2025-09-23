build/intermediate/cpp-0-strip/cpp.oe main.c -o main.ci
build/intermediate/cci-0-omc/cci.oe main.ci -o main.os
build/intermediate/as-1-compound/as.oe main.os -o main.oo
build/intermediate/ld-0-global/ld.oe build/intermediate/libc-0-oo/libc.oa main.oo -o main.oe
