#!/bin/bash

set -e

if [[ -z "$ONRAMP_BIN" ]]; then
    echo "Error: ONRAMP_BIN is not set or is empty." >&2
    exit 1
fi

pushd ../src/mkfs
./build.sh
popd

pushd ../src/os
./build.sh
popd

pushd ../src/ed
./build.sh
popd

$ONRAMP_BIN/onrampvm ../src/mkfs/mkfs ../sd.img \
    ../src/os/shell.oe:shell.oe \
    ../src/ed/ed.oe:ed.oe \
    \
    ../extra/build_toolchain.sh:build_toolchain.sh \
    ../extra/build-ccargs:build-ccargs \
    ../extra/main.c:main.c \
    ../extra/build.sh:build.sh \
    \
    $ONRAMP_BIN/../../intermediate/hex-0-onramp/hex.oe:hex.oe \
    \
    $ONRAMP_BIN/../../../core/ld/0-global/build.sh:core/ld/0-global/build.sh \
    $ONRAMP_BIN/../../../core/ld/0-global/ld.oe.ohx:core/ld/0-global/ld.oe.ohx \
    $ONRAMP_BIN/../../../core/libc/0-oo/build.sh:core/libc/0-oo/build.sh \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/start.oo:core/libc/0-oo/src/start.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/ctype.oo:core/libc/0-oo/src/ctype.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/environ.oo:core/libc/0-oo/src/environ.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/errno.oo:core/libc/0-oo/src/errno.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/malloc.oo:core/libc/0-oo/src/malloc.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/malloc_util.oo:core/libc/0-oo/src/malloc_util.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/spawn.oo:core/libc/0-oo/src/spawn.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/stdio.oo:core/libc/0-oo/src/stdio.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/string.oo:core/libc/0-oo/src/string.oo \
    $ONRAMP_BIN/../../../core/libo/0-oo/build.sh:core/libo/0-oo/build.sh \
    $ONRAMP_BIN/../../../core/libo/0-oo/src/libo-error.oo:core/libo/0-oo/src/libo-error.oo \
    $ONRAMP_BIN/../../../core/libo/0-oo/src/libo-util.oo:core/libo/0-oo/src/libo-util.oo \
    $ONRAMP_BIN/../../../core/ar/0-cat/build.sh:core/ar/0-cat/build.sh \
    $ONRAMP_BIN/../../../core/ar/0-cat/ar.oo:core/ar/0-cat/ar.oo \
    $ONRAMP_BIN/../../../core/as/0-basic/build.sh:core/as/0-basic/build.sh \
    $ONRAMP_BIN/../../../core/as/0-basic/as.oo:core/as/0-basic/as.oo \
    $ONRAMP_BIN/../../../core/as/1-compound/build.sh:core/as/1-compound/build.sh \
    $ONRAMP_BIN/../../../core/as/1-compound/src/emit.os:core/as/1-compound/src/emit.os \
    $ONRAMP_BIN/../../../core/as/1-compound/src/main.os:core/as/1-compound/src/main.os \
    $ONRAMP_BIN/../../../core/as/1-compound/src/op_arithmetic.os:core/as/1-compound/src/op_arithmetic.os \
    $ONRAMP_BIN/../../../core/as/1-compound/src/op_control.os:core/as/1-compound/src/op_control.os \
    $ONRAMP_BIN/../../../core/as/1-compound/src/op_logic.os:core/as/1-compound/src/op_logic.os \
    $ONRAMP_BIN/../../../core/as/1-compound/src/op_memory.os:core/as/1-compound/src/op_memory.os \
    $ONRAMP_BIN/../../../core/as/1-compound/src/opcodes.os:core/as/1-compound/src/opcodes.os \
    $ONRAMP_BIN/../../../core/as/1-compound/src/parse.os:core/as/1-compound/src/parse.os \
    $ONRAMP_BIN/../../../core/cpp/0-strip/build.sh:core/cpp/0-strip/build.sh \
    $ONRAMP_BIN/../../../core/cpp/0-strip/cpp.os:core/cpp/0-strip/cpp.os \
    $ONRAMP_BIN/../../../core/cci/0-omc/build.sh:core/cci/0-omc/build.sh \
    $ONRAMP_BIN/../../../core/cci/0-omc/src/common.os:core/cci/0-omc/src/common.os \
    $ONRAMP_BIN/../../../core/cci/0-omc/src/compile.os:core/cci/0-omc/src/compile.os \
    $ONRAMP_BIN/../../../core/cci/0-omc/src/emit.os:core/cci/0-omc/src/emit.os \
    $ONRAMP_BIN/../../../core/cci/0-omc/src/globals.os:core/cci/0-omc/src/globals.os \
    $ONRAMP_BIN/../../../core/cci/0-omc/src/lexer.os:core/cci/0-omc/src/lexer.os \
    $ONRAMP_BIN/../../../core/cci/0-omc/src/locals.os:core/cci/0-omc/src/locals.os \
    $ONRAMP_BIN/../../../core/cci/0-omc/src/main.os:core/cci/0-omc/src/main.os \
    $ONRAMP_BIN/../../../core/cci/0-omc/src/parse.os:core/cci/0-omc/src/parse.os \
    $ONRAMP_BIN/../../../core/cci/0-omc/src/type.os:core/cci/0-omc/src/type.os \
    $ONRAMP_BIN/../../../core/cpp/1-omc/build.sh:core/cpp/1-omc/build.sh \
    $ONRAMP_BIN/../../../core/cpp/1-omc/cpp.c:core/cpp/1-omc/cpp.c \
    \
    $ONRAMP_BIN/../../../core/libc/common/include/time.h:core/libc/common/include/time.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stdalign.h:core/libc/common/include/stdalign.h \
    $ONRAMP_BIN/../../../core/libc/common/include/semaphore.h:core/libc/common/include/semaphore.h \
    $ONRAMP_BIN/../../../core/libc/common/include/inttypes.h:core/libc/common/include/inttypes.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stdlib.h:core/libc/common/include/stdlib.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stdnoreturn.h:core/libc/common/include/stdnoreturn.h \
    $ONRAMP_BIN/../../../core/libc/common/include/malloc.h:core/libc/common/include/malloc.h \
    $ONRAMP_BIN/../../../core/libc/common/include/features.h:core/libc/common/include/features.h \
    $ONRAMP_BIN/../../../core/libc/common/include/limits.h:core/libc/common/include/limits.h \
    $ONRAMP_BIN/../../../core/libc/common/include/unistd.h:core/libc/common/include/unistd.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stddef.h:core/libc/common/include/stddef.h \
    $ONRAMP_BIN/../../../core/libc/common/include/wctype.h:core/libc/common/include/wctype.h \
    $ONRAMP_BIN/../../../core/libc/common/include/fcntl.h:core/libc/common/include/fcntl.h \
    $ONRAMP_BIN/../../../core/libc/common/include/signal.h:core/libc/common/include/signal.h \
    $ONRAMP_BIN/../../../core/libc/common/include/spawn.h:core/libc/common/include/spawn.h \
    $ONRAMP_BIN/../../../core/libc/common/include/setjmp.h:core/libc/common/include/setjmp.h \
    $ONRAMP_BIN/../../../core/libc/common/include/strings.h:core/libc/common/include/strings.h \
    $ONRAMP_BIN/../../../core/libc/common/include/sys/time.h:core/libc/common/include/sys/time.h \
    $ONRAMP_BIN/../../../core/libc/common/include/sys/types.h:core/libc/common/include/sys/types.h \
    $ONRAMP_BIN/../../../core/libc/common/include/sys/stat.h:core/libc/common/include/sys/stat.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stdckdint.h:core/libc/common/include/stdckdint.h \
    $ONRAMP_BIN/../../../core/libc/common/include/ctype.h:core/libc/common/include/ctype.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__seek.h:core/libc/common/include/__onramp/__seek.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__wint_t.h:core/libc/common/include/__onramp/__wint_t.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__size_t.h:core/libc/common/include/__onramp/__size_t.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__mode_t.h:core/libc/common/include/__onramp/__mode_t.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__time_t.h:core/libc/common/include/__onramp/__time_t.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__fatal.h:core/libc/common/include/__onramp/__fatal.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__null.h:core/libc/common/include/__onramp/__null.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__predef.h:core/libc/common/include/__onramp/__predef.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__bool.h:core/libc/common/include/__onramp/__bool.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__wchar_t.h:core/libc/common/include/__onramp/__wchar_t.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__timespec.h:core/libc/common/include/__onramp/__timespec.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__useconds_t.h:core/libc/common/include/__onramp/__useconds_t.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__pit.h:core/libc/common/include/__onramp/__pit.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__va_list.h:core/libc/common/include/__onramp/__va_list.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__arithmetic.h:core/libc/common/include/__onramp/__arithmetic.h \
    $ONRAMP_BIN/../../../core/libc/common/include/__onramp/__wchar_limits.h:core/libc/common/include/__onramp/__wchar_limits.h \
    $ONRAMP_BIN/../../../core/libc/common/include/wchar.h:core/libc/common/include/wchar.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stdbool.h:core/libc/common/include/stdbool.h \
    $ONRAMP_BIN/../../../core/libc/common/include/uchar.h:core/libc/common/include/uchar.h \
    $ONRAMP_BIN/../../../core/libc/common/include/iso646.h:core/libc/common/include/iso646.h \
    $ONRAMP_BIN/../../../core/libc/common/include/math.h:core/libc/common/include/math.h \
    $ONRAMP_BIN/../../../core/libc/common/include/errno.h:core/libc/common/include/errno.h \
    $ONRAMP_BIN/../../../core/libc/common/include/termios.h:core/libc/common/include/termios.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stdbit.h:core/libc/common/include/stdbit.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stdio.h:core/libc/common/include/stdio.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stdarg.h:core/libc/common/include/stdarg.h \
    $ONRAMP_BIN/../../../core/libc/common/include/assert.h:core/libc/common/include/assert.h \
    $ONRAMP_BIN/../../../core/libc/common/include/stdint.h:core/libc/common/include/stdint.h \
    $ONRAMP_BIN/../../../core/libc/common/include/string.h:core/libc/common/include/string.h \
    \
    $ONRAMP_BIN/../../../core/libo/0-oo/include/libo-error.h:core/libo/0-oo/include/libo-error.h \
    $ONRAMP_BIN/../../../core/libo/0-oo/include/libo-util.h:core/libo/0-oo/include/libo-util.h \
    \
    $ONRAMP_BIN/../../../core/cg/0-asm/build.sh:core/cg/0-asm/build.sh \
    $ONRAMP_BIN/../../../core/cg/0-asm/src/common.h:core/cg/0-asm/src/common.h \
    $ONRAMP_BIN/../../../core/cg/0-asm/src/instruction.h:core/cg/0-asm/src/instruction.h \
    $ONRAMP_BIN/../../../core/cg/0-asm/src/main.c:core/cg/0-asm/src/main.c \
    $ONRAMP_BIN/../../../core/cg/0-asm/src/opcode.h:core/cg/0-asm/src/opcode.h \
    $ONRAMP_BIN/../../../core/cg/0-asm/src/optimize.h:core/cg/0-asm/src/optimize.h \
    $ONRAMP_BIN/../../../core/cg/0-asm/src/parse.h:core/cg/0-asm/src/parse.h \
    $ONRAMP_BIN/../../../core/cg/0-asm/src/register.h:core/cg/0-asm/src/register.h \
    $ONRAMP_BIN/../../../core/cpp/1-omc/rebuild.sh:core/cpp/1-omc/rebuild.sh \
    $ONRAMP_BIN/../../../core/ld/1-omc/build.sh:core/ld/1-omc/build.sh \
    $ONRAMP_BIN/../../../core/ld/1-omc/ld.c:core/ld/1-omc/ld.c \
    \
    $ONRAMP_BIN/../../../core/libc/common/src/internal.h:core/libc/common/src/internal.h \
    $ONRAMP_BIN/../../../core/libc/common/src/syscalls.h:core/libc/common/src/syscalls.h \
    \
    $ONRAMP_BIN/../../../core/libc/1-omc/build.sh:core/libc/1-omc/build.sh \
    $ONRAMP_BIN/../../../core/libc/1-omc/src/malloc.c:core/libc/1-omc/src/malloc.c \
    $ONRAMP_BIN/../../../core/libc/1-omc/src/strrchr.c:core/libc/1-omc/src/strrchr.c \
    $ONRAMP_BIN/../../../core/libc/1-omc/src/strtol.c:core/libc/1-omc/src/strtol.c \
    $ONRAMP_BIN/../../../core/cc/build.sh:core/cc/build.sh \
    $ONRAMP_BIN/../../../core/cc/cc.c:core/cc/cc.c \
    \
    $ONRAMP_BIN/../../intermediate/ld-0-global/ld.oe:build/intermediate/ld-0-global/ld.oe \
    $ONRAMP_BIN/../../intermediate/ar-0-cat/ar.oe:build/intermediate/ar-0-cat/ar.oe \
    $ONRAMP_BIN/../../intermediate/libc-0-oo/libc.oa:build/intermediate/libc-0-oo/libc.oa \
    $ONRAMP_BIN/../../intermediate/libo-0-oo/libo.oa:build/intermediate/libo-0-oo/libo.oa \
    $ONRAMP_BIN/../../intermediate/as-0-basic/as.oe:build/intermediate/as-0-basic/as.oe \
    $ONRAMP_BIN/../../intermediate/as-1-compound/as.oe:build/intermediate/as-1-compound/as.oe \
    $ONRAMP_BIN/../../intermediate/cpp-0-strip/cpp.oe:build/intermediate/cpp-0-strip/cpp.oe \
    $ONRAMP_BIN/../../intermediate/cci-0-omc/cci.oe:build/intermediate/cci-0-omc/cci.oe \
    $ONRAMP_BIN/../../intermediate/cpp-1-omc-unopt/cpp.oe:build/intermediate/cpp-1-omc-unopt/cpp.oe \
    $ONRAMP_BIN/../../intermediate/cpp-1-omc-unopt/cpp.os:build/intermediate/cpp-1-omc-unopt/cpp.os \
    $ONRAMP_BIN/../../intermediate/cg-0-asm/cg.oe:build/intermediate/cg-0-asm/cg.oe \
    $ONRAMP_BIN/../../intermediate/cpp-1-omc/cpp.oe:build/intermediate/cpp-1-omc/cpp.oe \
    $ONRAMP_BIN/../../intermediate/ld-1-omc/ld.oe:build/intermediate/ld-1-omc/ld.oe \
    $ONRAMP_BIN/../../intermediate/libc-1-omc/libc.oa:build/intermediate/libc-1-omc/libc.oa \
    $ONRAMP_BIN/../../intermediate/cc/cc.oe:build/intermediate/cc/cc.oe \
