#!/bin/bash
set -euo pipefail
D="$(cd "$(dirname "$0")"&&pwd)"; R="$(dirname "$D")"
name="$1"; cd "$2"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 X0X_LIVE_TEST_OUT="$D/$name"
export X0X_LIVE_CASES="$D/extended-lifecycle.inc"
flags="$(cat "$R/all-controls-remote-join/compile-flags.txt") -DX0X_SUPERJ6_FACTORY=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_REMOTE_DNR3=1 -DX0X_REMOTE_TYPED_SYNTH=1 -DDNBOX_CHICAGO_MOD_PALETTE=1 -DDNBOX_MIAMI_STOCK_CAP_COMPACT=1 -DDNBOX_MIAMI_DECIMATOR_COMPACT=1 -DDNBOX_CHICAGO_MOD_SPEED_COMPACT=1 -DDNBOX_CHICAGO_DRIVE_COMPACT=1 -Wl,--wrap=engine_live_key"
SAN_FLAGS="$flags" sh tests/host/freeplay_live_keys_test.sh
