#!/bin/sh
set -eu
W=${1:-$PWD}
D="$W/perf-final-proof/sj6-fixed-provider"
T=${FIXED_SCRATCH:-$(cat "$D/checkout")}
F="$W/xip-lossless/rec-fw/experiments/sound-hardware-v1/sj6-factory"
printf '#!/bin/sh\nexec cc "$@" -lcrypto\n' > "$T/cc"
chmod +x "$T/cc"
CC="$T/cc" TUNING_SOURCE="$T/source" SMC_UI_TEST_FILE="$D/ui_fixed_provider_test.c" SMC_UI_OUT="$T/out" \
HOST_EXTRA="-I$D -I$T -I$T/source/tests/host -I$F -I$W/sj6-private-final/bench-work/compiled -Wl,--wrap=juno6_init" \
FIXED_HOST_IMAGE="$T/catalog.bin" sh "$W/perf-final-proof/sj6-picker-host-hook/replay-host.sh"
