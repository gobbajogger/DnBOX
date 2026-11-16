# Sound renderer implementation WIP

This checkpoint adds an actual coefficient-free procedural model renderer,
owner-supplied fitted coefficient assets and reproducible local tests. It does
not activate a new firmware bank, implement the complete original instrument,
qualify device timing or solve current combined memory admission.

The compact512 set is the default candidate: 16,752 coefficient bytes, 1,024
generated-cycle bytes per live model and native view/cursor sizes 64/2 bytes.
Six independent sets require 6,540 bytes before alignment, excluding coefficient
residency and the actual voice/filter/effect graph. The full2048 alternative is
26,148 coefficient bytes and 24,972 live bytes for six sets before alignment.
Compact saves 9,396 coefficient bytes and 18,432 cycle bytes against that larger
point, but has different measured approximation error. No captured PCM is stored
in either coefficient model. Generated cycles are mathematical RAM tables.

The actual native generic object has 5,294 executable bytes plus 8,192 bytes of
generic mathematical sine constants. Its only external symbols are cold memory
functions; this is object evidence, not a complete target link or timing result.
Cycles are constructed cold with work proportional to cycle length x harmonic
count. Audio-thread safety of construction is not established.

`assets/superj6/MODELS.json` records complete raw-record versus original fitter
played-span domains separately. Compact played-span null is about -57.65 to
-40.195 dBFS; full2048 about -66.85 to -54.45 dBFS. Onset peak errors remain large.
Neither number proves accepted musical reconstruction or full preset fidelity.

Run `sh tests/host/run_sj6_model_tests.sh`. It runs 1,204 protocol/position and
522 boundary/quantizer checks, then all 24 complete model vectors. The virtual
sample at frame_count is +0, matching the retained original interpolation guard;
the decoder does not incorrectly reject that compatible endpoint.

The new SynthOne adapter delegates real note, parameter, sequence/arp and render
operations to the recovered pinned source scheduler. It is pre-FX WIP and has
now passed a12-scene576,000-frame authenticated source-host pre-FX A/B with zero different channel words. This is not complete musical/device acceptance. It does not rename
Drone or reinterpret saved legacy bank2/3 data. The complete original FX,
150-parameter storage/admission, app wiring and 56-preset/full-FX and target original-source A/B remain
separate required work. Shared UI/runtime/storage files are untouched.

Source base is the verified 8a0ef30c family, not a qualified current composition.
All native/product pool, flash, stack, audio and timing gates remain in force.
Stock hardware only; no PSRAM, BT-off, shortened non-fitting delays or reverb
removal. Do not infer firmware readiness from the source checkpoint.
