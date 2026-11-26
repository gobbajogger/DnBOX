Host-only source topology test; synthetic marker executed in the real current
sampled Sub receiver. It has its own OTA input/history, measures exactly unchanged
filter input and output at physical half-fader; only final filter product moves.
Original separate Main and Sub nonlinear states are not mathematically
equivalent to the compact sum into one nonlinear memory. To isolate that
difference we made a one-owner host diagnostic with four additional Float32
states per active voice; existing compact carrier, generated phase, mode,
ADSR, fader names and values unchanged. Sub fader moved like Main only at the
real independent exit; no fader changes for Sub relative to authored normalized
input, no per-preset logic. Raw is scoped static unmodulated generated OTA
without Noise; trajectory grants no claims of live modulation/steal/retrigger.
Relative fixed-companion mixing mismatch improves by ~79–93 dB; unchanged
solo 27/53 quantized amplitude path, with exact Main fader including companion.
Full native Raw absolute/phase comparison barely moves and is not a sound fix.
Do not ship this test as a release patch.
Samples, protected profile arrays, and renders not present.

Use predecessor sj6-ota-source-fader-order source receipt, exact typed
donor and same compiler; wire-shared.py predecessorTyped copiedOut.
Build actual compose runtime, shared-driver.c, HOST_OTA_SPLIT_SUB=0/1 and
HOST_OTA_GAIN_AFTER=0/1, keep HOST_CENTER_SUM=0 and SJ6_WAVE_PROBE=1.
Archive includes control edit recipe and no private control values;
Raw full, Raw authored, Brainwave and Solidify use authentic unchanged projections.
sub-full: Raw minus OSC1; sub-half minus OSC1 and Sub code 27;
shared-a-half: Raw retained Sub plus OSC1 code64. Prior a-full/a-half/a-zero
from unchanged Raw Main-only variant. Output prefix s0/s1.
Run score-shared.py enclosingFoldScratch thisRun.
