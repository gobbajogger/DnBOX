# Dry Init: isolate and correct the Normal filter stage

The independent original ARM64 bench runs the older authenticated
SuperJ6 Normal coefficient setter and actual sample instructions, not
a Python filter approximation or an installed Audio Unit host. Drive a
controlled dry Init-like saw test at the same cutoff and damping, feed
the exact same signed samples and use no gain fitting.

The compact two-pole differs from the original recurrence by −19.46,
−11.57 and −4.86 dBr on the three test cases (4000, 1200 and 300 Hz).
The source-order double recurrence with Float32 I/O and feedback,
using coefficients returned by the original code, matches all 6144
signed samples bit for bit.

The guarded host patch wires that recurrence into the actual Init
instrument loop for a fixed 4 kHz, damping 1, dry, one-enabled-saw fixture.
It puts voice AMP before Normal, keeps each voice/layer's four separate
history words, and leaves the mixer and master untouched. The first 1024
runtime Main1 samples were fed back through the original instructions:
all 1024 output words matched. The host renders the whole exact 8-event
cue and repeats bit-identically. Flag-off preprocessing equals the donor.
The diagnostic startup is fixed-cutoff only; neither factory Init
defaults, dynamic cutoff nor end-to-end original sound are claimed.
No firmware image or PCM included; no target fit, timing or release claim.

Reproduce from the workspace root with existing pinned firmware-fit-next
and the project Unicorn 2.1.4 bench on PYTHONPATH, authorized older AU in
SJ6_BIN, and a new temporary directory T:

    python sj6-init-match/normal-stage/compare.py > /tmp/init-isolated.json
    bash sj6-init-match/normal-stage/build.sh firmware-fit-next "$T"
    python sj6-init-match/normal-stage/compare.py "$T/tap.f32"

Source gain and oscillator are separate next comparisons; do not label
the matched-filter result as matching either.
