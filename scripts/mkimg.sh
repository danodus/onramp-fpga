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
    \
    ../src/scripts/build_toolchain.sh:build_toolchain.sh \
    ../src/hello/hello.ohx:hello.ohx \
    ../src/ed/ed.oe:ed.oe \
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
    \
    $ONRAMP_BIN/../../intermediate/ld-0-global/ld.oe:build/intermediate/ld-0-global/ld.oe \
    $ONRAMP_BIN/../../intermediate/ar-0-cat/ar.oe:build/intermediate/ar-0-cat/ar.oe \
    $ONRAMP_BIN/../../intermediate/libc-0-oo/libc.oa:build/intermediate/libc-0-oo/libc.oa \
    $ONRAMP_BIN/../../intermediate/libo-0-oo/libo.oa:build/intermediate/libo-0-oo/libo.oa \
    $ONRAMP_BIN/../../intermediate/as-0-basic/as.oe:build/intermediate/as-0-basic/as.oe \
    $ONRAMP_BIN/../../intermediate/as-1-compound/as.oe:build/intermediate/as-1-compound/as.oe \
    $ONRAMP_BIN/../../intermediate/cpp-0-strip/cpp.oe:build/intermediate/cpp-0-strip/cpp.oe \
    $ONRAMP_BIN/../../intermediate/cci-0-omc/cci.oe:build/intermediate/cci-0-omc/cci.oe
