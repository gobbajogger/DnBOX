Source current native Main saw, request physical fader 0.808→0.404 and
invert the same generated oscillator before halfband. Both original
receiver leaf accumulators (actual final writes) match signed output.
Normal passes exact odd symmetry (and the physical input halves before its
linear IIR). OTA has intrinsic even component / sign asymmetry: max response
+X+−X .0020964 Main1, .00152312 Main2, neither is a postfilter bus asymmetry.
For exact half fader, same original OTA left filter input and output are
bit-identical before the postfilter multiply. At 0x1003b77e4 multiply operand
alone halves exactly (.52986519795→.26493259898 for the calibrated velocity/
sustain/fader), and both final output lanes match exact half word-for-word.
Taps at 0x1003b77e0→e4 capture active nonlinear helper. Right output
verified bitwise but separate asymmetric physical right-path call not reached by
these centered mono source cases. No Apple downstream bus audio claimed.
Native receipt merely controlled actual receiver, not a preset comparison.

True typed host narrow candidate moves only existing Compact Main fader
after its nonlinear OTA recurrence; the approx OTA model itself untouched.
Actual Raw Main-only stem driven by same saved chronological cue and strict
control edits, faders 1 and 3 tested independently at 1 and 64/127.
Postfilter-fader law error −47.99→−144.41 dBr pre-FX and −47.99→−143.05
final; signed two-Main sum/zero exact. Refuse shared Sub+noise to prevent
spectral/gain leakage: authentic complete Raw program (has Sub), Brainwave,
and Solidify path are byte-identical before/after, no full preset improvement
claimed. Exact placement can later be shared only with a genuinely separate
source-owned Sub path. No dynamic fader ramps or cutoff fitted. No shipping.

Reproduce: authorized current source host env, execute branch.py from repo
root with original emular packages; source receipt branch.json. For Compact
run wire.py priorFoldSumSource copiedCore, compile same typed host fixture
from branch close-pin at stock Fold32 (HOST_CENTER_SUM=0, SJ6_WAVE_PROBE=1),
toggle HOST_OTA_GAIN_AFTER=0/1; previous compose-driver/runtime. Generate
explicit controlled pairs with control_pairs.py; save actual typed pre/final
under 0/1 prefixes and score.py. No source media, compiled binaries, or audio.
