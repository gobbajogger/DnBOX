# Original-reference SuperJ6 source experiments

This is an incomplete source port, not firmware, a complete SuperJ6 engine or a usable device instrument. It changes no current engine, project/preset storage, physical UI or device. Preserve every original preset field and saved byte; the earlier approximate proxy is not the endpoint.

## Actual implemented reference scope

MIT AudioKit/DunneAudioKit base be581d08158ea4556a54298f75814d23ff7db886 is retained with credits/license. The source implements selected original sample/envelope/glide/phase/filter laws, Juno/SmoothPass, distinct nonlinear Syrup and four-stage/twice-oversampled OTA, the separate curvature3 filter envelope and original nonlinear gain placement. The selected analytic fallback saw/triangle/sine/PWM branches retain waveform priority, halfband histories/gain and original >0.5 boolean/PWM-width boundaries. This is not custom model/wavestack support. White noise takes precedence, with original global atomic seed-ticket/mix/xorshift/lifecycle laws; only DEFAULT coloration0stages/gain1/rate1 is covered.

Original start/stop clear analytic phase, increment and halfband histories. Actual deferred restart completion clears phase/history, but restart entry and legato retain them. An independent reviewer found the earlier producer's opposite lifecycle assumption; a direct authenticated-wide-store/source-only RED regression and the corrected timing are retained in the qualification lineage. Cold audio matches alone did not prove reuse.

Component32 is vintageFilterADSR, NOT isOSC30. Its separate log/Syrup and linear/key-warp cutoff laws are source-grounded. Captured32-enabled histories have audio filtersOFF, so they do NOT prove active-vintage PCM equality. Other filter reset/lifecycle semantics, particularly OTA signal history versus coefficient reset, remain unqualified. General arbitrary finite/nonfinite/alias/lifetime/concurrent API safety is not admitted.

The separate original-Saga timing core implements established stored-division/rate-knob distinction, snap epsilon, deterministic arp0..3, sequence-note transforms, at-most-one-step processing, gate-off-before-on, generated-release identity, core reset/rewind, pad targets/masks and increment-then-reset arp counter after held-pool shrink. Random4..6 and complete host/clock/pedal/preset/physical orchestration remain unresolved.

## Evidence and reproduction

Exact evidence identities and limits are in EVIDENCE.json. Forty-two privately recorded component histories compare942080 stereo frames per O2/Os/nonrecovering sanitizer variant. They use one real sample zone and narrow selected controls, NOT whole original presets, every mode or app FX. Each newly added Syrup/OTA family has one captured configuration. Original recordings came from a separately authorized restricted selected-routine ARM64 bench with guest network/file/syscalls blocked, unknown calls trapped and disclosed host math/runtime/single-thread bridges, not full iOS/Applelibm/concurrency identity. This public package includes no recordings or app bytes.

Run fixture-free source checks:

    python3 tests/run_host.py --out /tmp/unique-j6-source-check --manifest-sha256 EXACT_REVIEWED_MANIFEST_SHA256

Use the externally verified SOURCE-MANIFEST.json hash for this exact commit. The runner authenticates all package inputs before compiling and keeps outputs outside the immutable source. It runs actual voice-rate lifecycle paths, source-derived control/noise/vintage laws, the reset-timing regression, deterministic Saga tests and prior grouping/pool-shrink regressions. These do NOT replay private PCM, execute the original sequencer or prove a complete app graph. Default LeakSanitizer fails under ptrace in the recorded environment; same-binary detect_leaks=0 ASan/UBSan/float-cast passes are separate, NOT leak proof. Inherited import-deprecation warnings remain visible.

External test frontends require separately owner-supplied matching descriptors/data/plans. They are narrow caller-owned fixtures, not general file/preset importers. Source samples, state and writable output spans must be initialized, live and disjoint; never unload referenced data or render/control/load concurrently. Mode switching multiplies the supplied output buffers as the original does; do not use it over an already mixed shared bus.

## Not yet product admission

174 serialized preset IDs,218 static decoder/control IDs and52 selected component IDs are DIFFERENT domains. Full original preset-to-layer/asset/DSP/default/order routing, all models/noise coloration/control smoothing, modulation/full FX, original seq/arp/host-MIDI-Link clocks/FreeRunning/hold/pedal/note owners/preset locks and complete physical knob/pad workflows remain required. No unsupported value may be silently dropped.

Inherited allocations, vectors, static initialization and atomic/runtime dependencies are NOT allocation-free/realtime or current-platform ABI acceptance. Earlier genuine isolated filter diagnostics retain16 double helpers (six APIs) and4 (OTA float ingress), both numerical-policy FAILs. A separately preserved exact-OTA math experiment can remove its own Float-FMA/immediate-ingress dependencies without rewriting genuine binary64 equations; it is NOT wired into this sampler package or evidence that whole-sampler policy passes.

There is no whole-engine target executable, current-firmware link/resource fit, CPU/deadline/zero-dropout, complete preset listening, original iOS, device or release acceptance. SynthOne replacing Drone and safe saved-data migration remain separate unfinished integration work, not reinterpretation of legacy bytes. No app/AU, assembly, samples/waveforms/PCM, raw banks, SDK/toolchain contents, compiled outputs or private reports are included. Source preservation does not grant merge/release/device permission.
