#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
# cue.f32 is the authenticated authorized planar Warm Brainwave stage tape.
# sha256 bcfce38842c85e3e1aaf704824693a042ff513c68e824c1af2ecc189e1c88e9a
for f in equal-clock heterodyne;do cc -O2 -std=c11 -fPIC -shared -ffp-contract=off "$f.c" -lm -o "$f.so";done
cc -O2 -std=c11 -fPIC -shared -ffp-contract=off baselines/original.c -lm -o original.so
cc -O2 -std=c11 -fPIC -shared -ffp-contract=off -I baselines baselines/compact_probe.c -lm -o compact.so
OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 python guard.py
OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 python compare.py
