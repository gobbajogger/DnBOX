#!/bin/sh
set -eu
W=${1:-$PWD}
D="$W/perf-final-proof/s1-arp-clock-receiver"
C="$W/s1-audit-data/original-inputs/closure/scheduler/source-sequence"
P="$W/perf-final-proof/s1-arp-factor-diagnostic"
(cd "$W" && sha256sum -c "$D/REFERENCE.sha256" >/dev/null)
T=$(mktemp -d /tmp/s1-clock.XXXX)
trap 'rm -rf -- "$T"' EXIT
python3 "$D/adapt.py" "$C/source_sequence.hpp" "$T/adapted.hpp"
c++ -O2 -std=c++17 -ffp-contract=off -fno-fast-math -Wall -Wextra -Werror -Wno-reorder -I"$T" -I"$C" -I"$P" -I"$D" "$D/replay.cpp" -o "$T/oracle"
"$T/oracle"
