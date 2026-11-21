#!/bin/sh
set -eu
cd "$(dirname "$0")"
mkdir -p build logs
cc -std=c11 -O2 -ffp-contract=off -fno-fast-math -Wall -Wextra -Werror \
  -Wdouble-promotion -DX0X_HOST=1 -Ivendor -Isrc \
  src/s1_dry_voice.c vendor/dnbx_s1_envelope.c src/host.c \
  -o build/s1_dry_host >logs/build.stdout.txt 2>logs/build.stderr.txt
cc --version >logs/compiler.txt
nm -u build/s1_dry_host >logs/undefined_symbols.txt
