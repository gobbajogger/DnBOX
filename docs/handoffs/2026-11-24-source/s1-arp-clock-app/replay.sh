#!/bin/sh
set -eu
W=${1:-$PWD}
D="$W/perf-final-proof/s1-arp-clock-app"
B="$W/perf-final-proof/controls-init-only/source-s1-reverb-lfo"
T=$(mktemp -d "$D/temp.XXXX")
O=$(mktemp -d /tmp/s1-clock-app.XXXX)
trap 'rm -rf -- "$T" "$O"' EXIT
(cd "$B" && sha256sum -c ../S1-REVERB-LFO-POST.sha256 >/dev/null)
cp -al "$B"/. "$T/"
python3 "$D/apply.py" "$T"
run() {
    TUNING_SOURCE="$1" HOST_EXTRA="-DX0X_SYNTH_CONTROLS_INIT_ONLY=1 ${4:-} -I$1/tests/host" \
      SMC_UI_OUT="$2" SMC_UI_TEST_FILE="$3" sh "$W/perf-final-proof/sj6-picker-host-hook/replay-host.sh"
}
run "$T" "$O/app" "$D/app-test.c" -DX0X_S1_FENCED_CLOCK=1
mkdir "$O/active" "$O/idling"
CLOCK_TRANSPORT=1 SMC_UI_SHOTS="$O/active" "$O/app/test"
CLOCK_TRANSPORT=1 CLOCK_TRANSPORT_STOCK=1 SMC_UI_SHOTS="$O/idling" "$O/app/test"
cmp "$O/active/transport.pcm" "$O/idling/transport.pcm"
(cd "$O/active" && sha256sum transport.pcm)
legacy="$B/tests/host/ui_arp_legacy_pcm_test.c"
run "$T" "$O/exact-clock-idle" "$legacy" -DX0X_S1_FENCED_CLOCK=1
run "$B" "$O/old" "$legacy"
cmp "$O/exact-clock-idle/arp-legacy.pcm" "$O/old/arp-legacy.pcm"
(cd "$O/old" && sha256sum arp-legacy.pcm)
