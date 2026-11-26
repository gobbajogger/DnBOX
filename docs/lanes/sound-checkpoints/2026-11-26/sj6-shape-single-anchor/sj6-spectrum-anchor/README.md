# One-A4-Anchor model
This model calibrates relative harmonic amplitude and phase from exactly one
matched reference note for each shape. It never reads other notes during
generation and it does not read original row bytes or reuse original tables.
A4 manifests should name the signed projection convention and capture SHA.
Only the verified Normal20k/Q1 + HP10/Q1 + connected zero-Crusher + terminal DC
linear responses are removed alongside the verified 4× decimation response.
Fundamental carrier and receiver amplitudes remain independent of the measured
post-mix AU absolute level. No empirical transport offset is used.

Host stage performs original-source four successive binary32 phase updates,
real two-stage halfband and pitch/bend-selected row interpolation. For each
oversample it uses two table samples (or four during a cross-row blend).
Previous clock model used one lookup per output and no true decimator:
no performance numbers carry across. This version includes 16 halfband
history words in addition to existing phase, before integration/state-map work.

Commands:
  ./check.sh                         # synthetic smoke, not proof of sound
  python3 generate.py A4.json --size 64 > /tmp/anchor-generated.private.h
  cc -shared -fPIC -O2 -std=c99 -ffp-contract=off -I/tmp \
     -I../sj6-wavetable-scope -I../sj6-wave-carrier stage.c -lm -o /tmp/anchor.so
  python3 score.py /tmp/anchor.so frozen-holdout-projections.json

Private A4 anchor projections and headers are intentionally excluded from
source distribution. Retain only hashes and report metrics; score meaningful
signed waveforms separately from harmonic-only evidence.

There are 11 unique rows in each Q16 candidate: table bytes alone 1,408/2,816
for N64/N128, excluding row map, coefficients, executable instructions,
ABI growth, and differences in linked stock code. Duplicate rows in the
current host bench are intentional; runtime shipping not contemplated.
