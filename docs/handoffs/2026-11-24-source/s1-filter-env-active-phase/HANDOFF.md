# ID56 Synth One Filter Envelope LFO: active DSP proof only

Pinned original EffectsPanelController binds filterEnvLFOToggle to ID56.
Original portable voice evaluates descending LFO1, LFO2 or arithmetic mean
(BOTH), multiplies the filter-envelope amount and computes/clamps current
cutoff for the actual voice sample. The earlier held compact test showed
every 3,584 off-KR samples stale. This fenced host patch computes the actual
fresh cutoff only while selected LFO depth and envelope amount are active,
feeding the existing compact LP/BP/HP response. Original oscillator math,
filter mix, resonance/bandwidth and control tick scheduling unchanged.
Two already present coefficient fields unused by their respective filter
modes keep the held resonance (LP) and bandwidth reciprocal (BP);
no structure, pool, scratch audio state or note-lock allocation added.
The dependent compact coefficients change only for ID56 and use the same
per-filter transfer as their existing KR coefficient computation. Not
equivalence to original Soundpipe spectral/oscillator output or timing.

Real host physical Select/K4 obtains all four valid values in a generated
test-only FILTER MODE panel. That panel is not in the DSP patch, exists only
in the fixture and never changes project Sound or storage. Both owners,
three filter topologies and three LFO routes generate distinct actual wet
output, with every checked cutoff equal to original current-expression
oracle and 2,048 true filter attack + 2,048 release samples per trace.
Filter-mix-zero, Env-amount-zero and LFO-depth-zero are exact audio nulls.
Same-source untouched default stereo exact. No new bindings/bytes admitted.

Prior radix pack is no longer safe: exact current S1 base cell byte30
contains 5×5 FM/Decay × 3×3 Delay/Reverb gate =225 combinations. Adding
five independent absent/OFF/LFO1/LFO2/BOTH states to that byte needs
1,125 words and only 31 old-invalid byte values remain. No reuse of a
legacy opaque byte, no compile-default, native, preset or release claim.
Replay: bash replay.sh ../controls-init-only/source-mod-wheels.
