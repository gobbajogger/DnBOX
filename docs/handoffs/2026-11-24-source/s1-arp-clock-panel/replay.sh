#!/bin/sh
set -eu
W=${1:-$PWD}
D="$W/perf-final-proof/s1-arp-clock-panel"
B="$W/perf-final-proof/controls-init-only/source-s1-reverb-lfo"
T=$(mktemp -d "$D/temp.XXXX")
O=$(mktemp -d /tmp/s1-clock-panel.XXXX)
trap 'rm -rf -- "$T" "$O"' EXIT
cp -al "$B"/. "$T/"
(cd "$B" && sha256sum -c ../S1-REVERB-LFO-POST.sha256 >/dev/null)
patch -s --fuzz=0 --no-backup-if-mismatch -p1 -d "$T" < "$D/clock-panel-host.patch"
TUNING_SOURCE="$T" HOST_EXTRA="-DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_S1_FENCED_CLOCK=1 -I$T/tests/host" \
  SMC_UI_OUT="$O" SMC_UI_TEST_FILE="$D/panel-test.c" sh "$W/perf-final-proof/sj6-picker-host-hook/replay-host.sh"
mkdir "$O/active" "$O/idling"
CLOCK_TRANSPORT=1 SMC_UI_SHOTS="$O/active" "$O/test"
CLOCK_TRANSPORT=1 CLOCK_TRANSPORT_STOCK=1 SMC_UI_SHOTS="$O/idling" "$O/test"
cmp "$O/active/transport.pcm" "$O/idling/transport.pcm"
(cd "$O/active" && sha256sum transport.pcm)
