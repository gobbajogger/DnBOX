# SuperJ6 Build9 physical Crusher control converter

Exact scope: canonical public Float32 q/127 (q=0..127), not arbitrary
continuous AU values, modulated ranges, or project migration. converter.h
validates exact input bits using existing unit_control's Q31 mapping. Rejects
NaN, negative zero, mismatched word, unsupported ID, unsigned nonendpoint
#107 words and out-of-grid inputs without modifying state. Connected graph
and actual power-era proof are external, not derivable from the output tuple.

oracle.py executes pinned original Build9 conductor and actual AKDecimator
receiver through the Apple ABI, matched against the source formulas on all
256 input words. OS runtime and pow are substituted only at their imports
(as in the earlier receiver). No audio/remote/native playback claim.
Fastmath reference baseline differs by -2..2 and -1..4 ULP;
compose.py packs these signed residuals with per-row bias into 96 B.
Converter applies exact word adjustment, no runtime pow/double/extra table
levels. Commit returns touched register mask: #107 resets wet and Booster,
#108 changes wet by itself, #109 changes cadence and explicitly requests
rounding-zero, with all untouched members preserved. It is control-rate only.

Exact warm headers: fastmath.h SHA256 c82737d5302d77b974d5071c9bc58915aca723a6e6f5f5185cbe43bad962189d;
unit_control.h SHA256 a9cbebb4b9e5e99cd16a029e857139840a8f98e39790687f563460cea6e6e995.
Host -ffp-contract=off -fno-fast-math at O0/O2/Os matched 256/256 output
words and verified change/negative tests. Vendor PI32v2 standalone object
uses -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Oz -ffreestanding:
probe wrapper 322 B code and 96 B table, plus emitted weak shared
fastmath functions (do not sum these into current image; no LTO link).
No unresolved libm/soft-double symbols. No target execution/paired link;
this is not a production fit or enablement.

Replay: put pinned ARM64 and macho.py plus Unicorn/Capstone on private
workspace; run oracle.py ARM64 >oracle.json. Compile probe.c with pinned
headers (app/ and dsp/) and -DX0X_HOST to regenerate base-grid.txt; run
compose.py >correct.generated.h; compile and run test.c and compare printed
grid to oracle.json:grid. The archive contains only authored evidence and
public mapping labels, no original binary, PCM, or preset values.
