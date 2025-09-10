#!/bin/bash

set -e

if [[ -z "$ONRAMP_BIN" ]]; then
    echo "Error: ONRAMP_BIN is not set or is empty." >&2
    exit 1
fi

$ONRAMP_BIN/onrampcc -I ../lib mkfs.c -o mkfs
