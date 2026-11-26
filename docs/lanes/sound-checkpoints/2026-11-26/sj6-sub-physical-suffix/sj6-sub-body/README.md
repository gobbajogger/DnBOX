# Brainwave Sub held spectral phase explained by downstream transfer

Uses authorized old sample media decoded only in process memory, current
actual ARM64 DSP unmodified, the original verified physical control
projection, and official installed signed Sub-only AUv3 terminal render.
No native audio or old sample words are copied out. No output level trim,
waveform alignment, preset fit, new handoff implementation or production.

At original note 48→ranged key36 and later chord final 67→ranged 55 in
asset root54, observe actual sampler's pre/post Normal per-voice output.
Post-filter words equal the raw current compiled receiver on both windows.
Measure each steady state at its own identified frequency; this is
*spectral shape (relative to H1)*, not an absolute signed level/phase pass.
Forward chronological known linear HP10/Q1 → connected active-amount-zero
Crusher (pole .995 F32) → terminal DC (.999 F32) on first channel. The
proven scalar stages are irrelevant to relative Fourier phase and level.
No approximation to live current Swift cutoff/chord modulation is asserted.

First held: incoming root36 loop-body and sample start H3 −174.67°.
Original sampler pre/post Normal H3 −174.67/−174.73°. Passing chronological
post through known linear stages gives H3 +135.26° after HP, +13.60° after
Crusher, −18.67° after terminal DC. Installed H3 is −18.71° in both
repeats. H5 is predicted +16.481° vs observed +16.496°. H3/H5 component
levels within .052/.163 dB. The original shape's terminal phase is not its
carrier prefilter shape, and no arbitrary oscillator rotation is needed.

Second held from asset root54 (played at shifted key55 with source glide):
raw asset body H3 −120.54°. Current receiver pre/post −132.43/−133.83°.
Known output chain predicts +146.95°, installed +148.06°; residual H5
phase ~6.56° and level ~1.72 dB show incomplete current source rotation/
frequency/linear projection. Do not call tail/full parity, or infer a
signed startup match from a time-invariant held shape diagnostic.

Rebuild at workspace root with sj6-sub-handoff/probe.py and authorized
latest oracle pointers already sourced, exact pinned readback archive,
prior receiver control receipt and existing Unicorn/SciPy. Run:
  PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench \
  OPENBLAS_NUM_THREADS=1 python3 sj6-sub-body/probe.py > result.json
No original programs, samples, PCM or derived reconstruction coefficient
tables are included. Only rounded stage/scoring observations and code.
