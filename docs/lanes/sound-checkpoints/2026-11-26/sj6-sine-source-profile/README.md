# SuperJ6 generated Sine profile: guarded source stage only

`bash run.sh [exact donor DSP directory]` checks pinned dnbx_core.c, regenerates the patch, separately adds a measurement-only tap, compiles guard off/on, and checks measured RMS/projected h1 and refusal digests. Candidate.patch excludes the probe. The two generated signed phase projections are included in logs (cos/sin per oscillator), but the authenticated original results available for this comparison are amplitudes, not independently aligned signed PCM; no real original waveform null claimed.

Exact current-original constructor profile with normalized source Sine: Float32 1.2377909421920776, hex 0x1.3cdfdep+0. No whole-instrument gain or history change. The original compact factory shape direct .5 amplitude and .808 fader did not absorb that profile. Fold only the typed generated Sine receiver fader into the existing per-span osc_gain before the sample loops. Typed authored faders ≤.808 already take the hot non-unity multiply; the unprojected/default unity 1 bypass is deliberately refused, so it does not enter extra hot operations. No new state, voice or preset-specific behavior. Legacy Sine, triangle, saw, pulse, muted and silent digests exact; with mixed Sine+saw the sibling measured source exact. The active Sine can drive different filter inputs later as intended; later absolute gains and clipping still unverified.

Actual real typed warm A4 at frame 16384 and velocity87, two rated comparisons over 0.5s held reference:
- 44.1k raw each .195695936015→.242230644262 RMS vs current-original warmed .241990942 (+.00860 dB candidate). Dual coherent combined .484461288524.
- 48k raw each .195696036879→.242230769059 RMS; h1 .276755980752→.342566028052 vs current-original warmed 48k h1 .34219668 (+.00937 dB). Dual coherent combined .484461538118.
- Guard is host-only/off by default. No release or hardware change, no target timing or full sound judgement.
