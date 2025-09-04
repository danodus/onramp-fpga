#!/bin/bash

set -e

if [[ -z "$ONRAMP_BIN" ]]; then
    echo "Error: ONRAMP_BIN is not set or is empty." >&2
    exit 1
fi

pushd ../src/mkfs
./build.sh
popd
$ONRAMP_BIN/onrampvm ../src/mkfs/mkfs ../sd.img $ONRAMP_BIN/../../intermediate/hex-0-onramp/hex.oe ../src/hello/hello.ohx
