# SuperJ6 compatibility experiment: original proxy, not app DSP

This isolated experiment makes an incomplete, explicit approximation playable
without replacing the current Juno engine or pretending to have AudioKit's
proprietary model, oscillator, effects or parameter laws. It is not firmware,
an exact SuperJ6 port, app-reference audio fidelity or a deployable product.

## Two different kinds of preservation

The unchanged public PR909 parser and transport at
`3eaf1ad2d4de3c181e628916f64a3186baf0e7c7` preserve original bank bytes and
all parameter values, ordering, identifiers, UID and unknown fields.
`projection.py` consumes its inspected Decimal/int preset objects and retains
a detached decoded-source copy. That copy alone is NOT raw-byte restoration.
No bank, envelope containing a bank, patch export, app, waveform or audio is
included in this source package.

Approximation requires an explicit acknowledgement and independent choices
between `neoncutoffLayer1`/`neonfilterCutoff` and
`neonresonanceLayer1`/`neonfilterRez`. There is no hidden priority.
Every input parameter is classified as approximately mapped or unsupported.
The result always says `projection_complete=false` and
`APPROXIMATE_NOT_J6_DSP`. An unsupported field is preserved, not implemented.

All 640 supplied presets use one of 14 observed version strings. This is
neither an official schema nor the app's 0.999.8 version. Unknown versions are
refused by the proxy projection, not by exact transport. Both amp ADSR quartets
are mapped separately as an explicit experimental design choice. Their actual
layer routing is unverified: 29 presets disagree despite amp-link being on.
No link flag is interpreted. Filter ADSR stays separate.

## Declared approximate equations and defaults

The physical-unit C patch has three independent envelopes (amp1, amp2, filter).
Normalized attack maps to .002+3*x*x seconds, decay to .01+3*x*x, release
to .01+4*x*x and sustain to x. These are our chosen curves, not J6 laws.
Chosen cutoff maps to 35+12000*x*x Hz, resonance to .6*x, filter amount to
7000*x Hz. Saw/pulse/sub/noise and output are independent approximate gains.
Saw/pulse values through 1.001 and output through 1.32 are retained, never
silently clipped to 1. Every mapped domain is checked; unsupported ranges fail.

Manual original-engine defaults are equal layer mix, identical tuning, .5
pulse widths, no high-pass or LFO modulation. They are NOT discovered app
defaults. Pitch, oscillator model/waveform/transposition codes, amp/filter link
semantics, effects, stereo, mono/legato/unison/glide, arp/sequencer, tuning
and app numerical LFO destination/waveform/mode codes remain unsupported.
Three target LFOs accept ONLY this engine's named PWM/pitch/cutoff/amp masks;
the importer never translates app numeric route codes.

`sj6x.c` contains original analytic saw/pulse oscillators with polynomial
edge correction, square sub, deterministic xorshift noise, two-pole low-pass,
optional one-pole high-pass, linear segmented independent ADSRs and three
triangle LFOs. Six voices use deterministic free/retrigger/round-robin stealing.
Velocity-zero means note-off. Attack restarts from zero on retrigger; note-off
captures a linear release slope. Changing a release parameter does not rewrite
an already captured slope. No ML model, original app code or effects are used.
Its gain is /6 for headroom, deliberately NOT the current old Juno's /1.85.
This difference is not a recovered regression or app gain law.

Oscillator increments are bounded to [.000001,.45] cycles/sample; sub to half.
PWM, modulation amplitude and filter coefficients have explicit safety limits.
These are ORIGINAL engine behavior, not silent source-preset normalization.
Control changes are block-boundary changes. No heap/cache/global mutable state;
caller owns the complete state. The header defines mappedness/alignment/alias
and no-concurrent-forged-state obligations. Invalid public arguments return
before mutation. This is not arbitrary-hostile-object or OS mapping validation.

## Reproduce the bounded checks

`python3 -B run_tests.py --output NEW_DIRECTORY` runs the synthetic Python
suite and strict binary32 C O2/Os host suites, plus ASan/UBSan/float-cast checks.
Default LSan may fail under ptrace; the driver retains that failure and runs
the SAME binary with detect_leaks=0 separately. Leak-disabled pass is not leak
acceptance. The runner uses installed trusted cc, not arbitrary downloads.

To add the private corpus check, pass `--archive PATH --reference PATH`.
The reference directory is the unchanged PR909 `validator` directory; both
Python dependencies are hash checked. The exact authorized archive digest is
checked before reading. No preset/audio bytes are written. The harness builds
its own trusted source library and tests every preset with all four explicit
cutoff/resonance policy combinations. Standalone `run_corpus.py --library`
loads executable code: only supply a trusted exact-built ABI-matching library,
never a downloaded or arbitrary path. Its input is not authenticated by CDLL.
ctypes structure sizes match the C API; independent/native layout is still a
separate check, not a portability promise across unrelated ABIs.

## What the current results establish

Producer and independent host runs preserve all 640 presets using unchanged
PR909 restoration, produce 2560 incomplete proxy projections and 655360 finite
proxy samples, retain all 29 conflicting amp quartets and explicitly report
154 parameter IDs unsupported. Producer O2/Os test 2150 checks each.
An independently authored extreme/all-LFO/sixvoice/partition/release suite
checks 2173632 assertions at 8k,44.1k,48k,96k. These are actual proxy outputs,
NOT comparison with the original app or full hard-preset qualification.

The exact existing pi32v2 r3/FPU compiler compiles the source. A bare API
link fails on memcpy, honestly retained. Linking the REAL project freestanding
libc at public commit `035026fe70059af35e25c7c121b40f318b732fe3`,
`firmware/src/libc.c` blob `40628a590ae4e130402c16a292e1b11629d1d1ee`,
succeeds for all seven APIs with no undefined or soft-double symbols.
The API-only ELF has 2820 bytes executable text plus 48 readonly bytes, no
data/BSS; caller-owned target state is 612 bytes, patch144, voice72, align4.
This is NOT a firmware linker map, stack/cycle measurement, whole-product fit,
sample/FX/Juno/SMC/UI composition or hardware execution. No region is licensed
for asset placement by section sums.

The actual J6 sound identity remains unresolved. Official feature descriptions
at https://audiokitpro.com/synthonej6/ do not supply original DSP equations or
a source/model redistribution license. The older AudioKitSynthOne and our
current original Juno are not the J6 app. Safe static IPA inspection is not
execution or a source license. Proprietary app/bank bytes stay excluded.

## Next gates

Independently preserve this exact plain source; obtain legitimate original
J6 source/parameter-law or matched reference-audio evidence; implement and
label exact/approximate/unsupported features separately. Measure all materially
different heavy configurations and identify which interactions raise load.
Product integration, full native resource/CPU, zero-discontinuity/lost-event,
UI design and device/release gates belong to the program owner.
The separate Synth One compact-reader/metadata-only/PCM lane remains separate.
Chicago admission and the unattributed older J6 mapping claim remain unknown.
