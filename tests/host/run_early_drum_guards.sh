#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
tmp=$(mktemp -d)
trap 'rm -rf -- "$tmp"' EXIT
for t in lease mailbox; do
    "${CC:-cc}" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
        -Ifirmware/hal -Ifirmware/src/app "tests/host/early_drum_${t}_test.c" -o "$tmp/proof"
    "$tmp/proof"
done
