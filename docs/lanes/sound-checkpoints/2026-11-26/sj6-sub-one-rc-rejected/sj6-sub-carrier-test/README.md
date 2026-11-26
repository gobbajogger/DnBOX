# One-state causal Sub carrier recoloration rejected

Test a physically motivated fixed analog RC color, not an edited factory
preset, model extrapolated from per-preset results, waveform fitting or newly
extracted coefficient bank. Bare baseline is the existing compact Sub
harmonic law, built in-process on its source receiver's exact sample clock and
source media positions. Candidate adds 1 ms globally fixed causal RC
before playback. The engineering test value is one transparent fixed
prototype, not an inferred claim about original RC hardware. Its live
storage would be one float per active Sub voice, coefficient prepared
from rate once, but no target work was performed or needs to follow this fail.

All three arms run the exact current compiled sampled receiver with same
SFZ zone map, sample source rates and lengths, incoming note/velocity order,
envelopes, fader, clocks and callback splits. Original private media and
generated test data only in process memory. No RCA normalization, timing
shift, polarity correction, terminal gain, source HP or DC changes.
The program compares the actual receiver output, not installed terminal
absolute gain. It validates the 49875 commit and 55 last requested key,
with no phase fitting. Retains other source controls and no key interpolation
to repair sound artificially.

Lower root36 worsens signed onset/held/release +4.425/+4.200/+4.147 dBr to
+4.674/+4.558/+4.530. After the distinct root54 handoff, all three windows
get ~1.85–1.90 dB better but remain >+3.3 dBr (signed residual still exceeds
reference RMS). Pending before commit also worsens -1.595→-.540 dBr.
Held-out played MIDI 72 worsens onset/held/release −5.097/−6.124/−6.935 dBr
to −4.036/−3.112/−2.886, while held level falls ~7 dB below observed source.
Do not adopt. Simple global RC cannot repair distinct signed source media
states. No claim about long-loop boundary precision (these cues sample the
lead-in and held segment only), real target pressure, or endogenous source
analog transfer. No extra normalization used to hide rejection.

Run at workspace root:
  PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench \
  OPENBLAS_NUM_THREADS=1 python3 sj6-sub-carrier-test/probe.py > result.json

Requires prior authenticated sj6-sub-handoff/probe.py and current installed
original compiled Core pointer, only read from authorized workspace. No
original data/old sample/protected executable or PCM in deliverable.
