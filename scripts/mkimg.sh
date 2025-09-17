#!/bin/bash

set -e

if [[ -z "$ONRAMP_BIN" ]]; then
    echo "Error: ONRAMP_BIN is not set or is empty." >&2
    exit 1
fi

pushd ../src/mkfs
./build.sh
popd
$ONRAMP_BIN/onrampvm ../src/mkfs/mkfs ../sd.img \
    ../src/hello/hello.ohx:hello.ohx \
    $ONRAMP_BIN/../../intermediate/hex-0-onramp/hex.oe:hex.oe \
    $ONRAMP_BIN/../../../core/ld/0-global/ld.oe.ohx:core/ld/0-global/ld.oe.ohx \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/start.oo:core/libc/0-oo/src/start.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/ctype.oo:core/libc/0-oo/src/ctype.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/environ.oo:core/libc/0-oo/src/environ.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/errno.oo:core/libc/0-oo/src/errno.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/malloc.oo:core/libc/0-oo/src/malloc.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/malloc_util.oo:core/libc/0-oo/src/malloc_util.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/spawn.oo:core/libc/0-oo/src/spawn.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/stdio.oo:core/libc/0-oo/src/stdio.oo \
    $ONRAMP_BIN/../../../core/libc/0-oo/src/string.oo:core/libc/0-oo/src/string.oo \
    $ONRAMP_BIN/../../../core/libo/0-oo/src/libo-error.oo:core/libo/0-oo/src/libo-error.oo \
    $ONRAMP_BIN/../../../core/libo/0-oo/src/libo-util.oo:core/libo/0-oo/src/libo-util.oo \
    $ONRAMP_BIN/../../../core/ar/0-cat/ar.oo:core/ar/0-cat/ar.oo \
    # \
    # $ONRAMP_BIN/../../intermediate/ld-0-global/ld.oe:build/ld-0-global/ld.oe \
    # $ONRAMP_BIN/../../intermediate/ar-0-cat/ar.oe:build/ar-0-cat/ar.oe \

