#!/bin/bash
set -euo pipefail
w=$(realpath "$1"); cd "$2"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
. tests/host/original_build.sh
cc -O1 -pipe -std=c11 -DX0X_HOST -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DDNBOX_137_HOST_WITNESS=1 -DX0X_SJ6_MONO32_PROBE=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 $ORIGINAL_FLAGS -Itests/host -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen -Iout -I"$PWD" tests/host/voice-seed-project.c $ORIGINAL_SOURCES -lm -o "$w/export"
"$w/export" gate "$w/gate-project.bin"
"$w/export" voice "$w/voice-project.bin"
sha256sum "$w/"{gate,voice}-project.bin
