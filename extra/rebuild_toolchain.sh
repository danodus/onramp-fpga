# Copyright (c) 2025 Daniel Cliche
# SPDX-License-Identifier: MIT

# echo ====== Clean output
# rmall build/output/

echo ====== Install the libc headers
core/libc/common/build.sh

# echo ====== Rebuild our C toolchain with itself
# core/libc/3-full/rebuild.sh
# core/libo/1-opc/rebuild.sh
# core/cc/rebuild.sh
# core/ld/2-full/rebuild.sh
# core/as/2-full/rebuild.sh
# core/cci/2-full/rebuild.sh
# core/cpp/2-full/rebuild.sh

# core/hex/1-c89/build.sh

# cp build/intermediate/ar-0-cat/ar.oe build/output/bin/ar.oe
