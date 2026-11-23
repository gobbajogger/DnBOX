#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# H1-H5 arp hooks on the whole host app (Synth One patch + SuperJ6 factory profile).
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/synth-arp-hooks}
mkdir -p "$out"
sh host/build_host.sh --generate-only >"$out/assets.log" 2>&1
export X0X_SUPERJ6_FACTORY=1
. tests/host/original_build.sh
${CC:-cc} -std=c11 -O1 -g -DX0X_HOST -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE \
    -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
    -Wl,--wrap=s1_part_note -Wl,--wrap=juno6_note_on -Wl,--wrap=juno6_note_off -Wl,--wrap=juno6_init \
    $ORIGINAL_FLAGS ${SAN_FLAGS:-} tests/host/synth_arp_hooks_test.c $ORIGINAL_SOURCES -lm -o "$out/test"
"$out/test"
