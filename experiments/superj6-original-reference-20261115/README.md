# Original-reference SuperJ6 source experiments

This source is an incomplete port experiment, not firmware, a complete SuperJ6 port, or a usable device engine. It replaces no current engine and changes no project or preset storage. Every supplied preset field and original save must remain preserved until its actual behavior and migration are implemented and verified.

## What is implemented

The sampler starts from MIT AudioKit/DunneAudioKit commit be581d08158ea4556a54298f75814d23ff7db886. It includes source-grounded original-compatible envelope FMA, continuous legato glide, sample phase and filter preparation laws, plus the original Juno/SmoothPass filter family and its five-millisecond fade/reset transition. Voice rate changes preserve SmoothPass histories/current coefficients while updating targets. Full external API safety and original mode combinations are not admitted.

The timing component implements the statically established original Saga stored-division/rate-knob distinction, snap epsilon, deterministic arp0..3, sequence-note transforms, at-most-one-step processing, gate-off-before-on order, generated-release identity, transport/rewind core laws, pad target bindings and destination masks. It preserves unresolved random modes explicitly rather than inventing their behavior. The original constructor division .5 differs from applying rate knob0. Arp selection uses modulo, but its postselection counter increments then resets0, including when held pools shrink.

## Evidence and reproducible checks

Source versions and qualified checks are listed in EVIDENCE.json. Twenty-five recorded original-component histories were compared privately, not distributed here: 872448 stereo frames per O2/Os/nonrecovering sanitizer variant. These include one real sample zone, not whole original presets, every oscillator/filter mode or full app effects. Original recordings used a separately authorized restricted selected-routine ARM64 bench with guest network/file/syscall access blocked, unknown calls trapped, no app install/UI, and disclosed host math/runtime/single-thread lock bridges. They are not full iOS, Applelibm or concurrency identity.

The public tests are intentionally fixture-free. They cover the three real voice rate-update lifecycle paths and deterministic/static-derived Saga behavior and review regressions. They do NOT repeat the private recorded-PCM comparison and do not claim original executed sequencer timing. Run:

    python3 tests/run_host.py --out /tmp/unique-j6-checks --manifest-sha256 EXACT_SOURCE_MANIFEST_SHA256

Use the externally verified SHA256 of SOURCE-MANIFEST.json from the reviewed commit. The runner authenticates all package source before compiling and keeps output outside this tree. LeakSanitizer can fail under ptrace; a same-binary detect_leaks=0 pass is reported separately and is not leak proof. Inherited #import compiler deprecation warnings remain visible, not hidden.

The two external test frontends require owner-provided matching descriptors, waveform and recorded-plan inputs that are not included. They are narrow fixture drivers with caller-owned valid lifetimes, not a general audio-file/preset importer. Source samples, state and both writable output spans must be valid, live and disjoint. Do not unload while voices reference data or call render/control/load concurrently. A mode transition multiplies the supplied channel buffers as the original does; do not apply this directly over an already mixed shared bus.

## Still required for complete original ports

SuperJ6's174 preset IDs,218 static decoder/control IDs and52 selected sampler-component IDs are distinct domains, not counts of implemented controls. Complete original preset-to-layer/asset/DSP routing, all waveform/noise/filter variants, modulation, full FX, parameter smoothing, original own sequencer/arp orchestration, host/MIDI/Link clocks, Free Running, hold/pedal/note-owner reconciliation, preset locks, pad cancellation/origin adapters and physical knob/pad integration remain required. Absolute pitch-class math here is not the full key/scale/tuning adapter. Random4..6 remain unresolved. No unsupported field may be silently dropped.

The vector/value-copy timing model and inherited sampler allocations are not allocation-free or realtime-safe. There is no whole-engine target link, double-math policy pass, complete memory/flash fit, CPU deadline, zero-dropout, device or full-preset listening acceptance. Preserve the current integration owner and exact numerical/storage/resource gates. SynthOne replacing Drone remains a separate safe persistence/integration outcome, not a reinterpretation of old saved bytes.

No original app/AU binary, assembly, private sample, waveform/PCM, preset bank, SDK/toolchain, compiled output or private report is included. AudioKit/Dunne credits and the MIT license are retained. This source preservation is not permission to distribute excluded third-party bytes or to merge, release or operate hardware.
