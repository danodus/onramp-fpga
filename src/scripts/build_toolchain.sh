# Copyright (c) 2025 Daniel Cliche
# SPDX-License-Identifier: MIT

echo ====== Clean everything
rmall build/

echo ====== Copy our pre-built hex tool
cp hex.oe build/intermediate/hex-0-onramp/hex.oe

echo ====== Get our linker and libc up
core/ld/0-global/build.sh
core/ar/0-cat/build.sh
core/libc/0-oo/build.sh

echo ====== Build an assembler
core/libo/0-oo/build.sh
core/as/0-basic/build.sh
core/as/1-compound/build.sh

echo ====== Build our omC compiler
core/cpp/0-strip/build.sh
