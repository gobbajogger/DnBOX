#!/bin/sh
set -eu
cd "$(dirname "$0")"
mkdir -p build logs
cc -std=c11 -O2 -ffp-contract=off -fno-fast-math -Wall -Wextra -Werror \
  -Wdouble-promotion -DX0X_HOST=1 -Ivendor -Isrc \
  -c src/s1_dry_voice.c -o build/s1_dry_voice.o \
  >logs/build-core.stdout.txt 2>logs/build-core.stderr.txt
cc -std=c11 -O2 -ffp-contract=off -fno-fast-math -Wall -Wextra -Werror \
  -Wdouble-promotion -DX0X_HOST=1 -Ivendor -Isrc \
  src/s1_dry_voice.c vendor/dnbx_s1_envelope.c src/host.c \
  -o build/s1_dry_host >logs/build-host.stdout.txt 2>logs/build-host.stderr.txt
cc -std=c11 -O2 -ffp-contract=off -fno-fast-math -Wall -Wextra -Werror \
  -Wdouble-promotion -DX0X_HOST=1 -Ivendor -Isrc -Itests \
  tests/scalar_test.c vendor/dnbx_s1_envelope.c \
  -o build/scalar_test >logs/build-scalar.stdout.txt 2>logs/build-scalar.stderr.txt
cc --version >logs/compiler.txt
nm -u build/s1_dry_voice.o >logs/core-undefined-symbols.txt
nm -u build/s1_dry_host >logs/host-undefined-symbols.txt
