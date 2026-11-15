# Exact F32 math and original-OTA reference experiment

Plain source, tests and documentation only. This does not wire a firmware engine, replace a public API, or admit a complete SuperJ6 port.

The original OTA class keeps its double-parameter ABI, all fields, constants, operation order and every other cmath operation. Its36 already-binary32 FMA sites use an exactly single-rounded F32 primitive; its two immediate binary64-to-binary32 input casts use an integer representation adapter. This is not a general software-double emulator or permission to rewrite unrelated genuine binary64 equations. Reversing these substitutions reproduces the baseline header byte-for-byte. Source derivation and pins are retained.

The FMA/conversion contract is fixed round-to-nearest, ties-to-even, gradual subnormals and tininess-after rounding with explicit per-call flags. NaNs canonicalize to positive quiet0x7fc00000; negative zero and infinity signs are retained. The float/double ABI adapters do NOT read or change ambient rounding modes or exception flags. Thus arbitrary FENV modes and original ARM NaN payload/exception behavior are NOT equivalent. Controlled finite-RNE callers are the qualified scope. This caller-owned reference API is not a general alias/lifetime/parameter-safety or realtime admission.

Berkeley SoftFloat3e primary commit07ebd29db0cfb55723a79fbbffe99103ef38e305/treeb496c8187bd6e6111166ace40eb4323a0ed2573a supplies the integer arithmetic. Full BSD3 notices remain in generated code and COPYING.SoftFloat.txt. AudioKit/Dunne MIT attribution remains separately in LICENSE.AudioKit. No license to distribute original app samples, banks or binaries is inferred.

Run fixture-free host checks with Python3 and C/C++ compilers:

    python3 verify_host.py --out /tmp/unique-ota-check --manifest-sha256 EXACT_REVIEWED_MANIFEST_SHA256

Use the externally verified SOURCE-MANIFEST.json hash for the exact commit. The runner authenticates all package inputs before creating output and stages byte-identical copies outside the immutable package. It runs the unchanged OTA/control/conversion suites, an arbitrary-integer conversion oracle and deterministic generation checks. It needs no app, original recordings or sample/preset assets. Do not compile math/fma32.c separately when compiling numeric_boundary.c: the latter intentionally includes the unchanged FMA unit to share its local rounding machinery. Included native_api.cpp is only a reproducible API-retention frontend; the host runner does not perform native builds.

Fresh controlled host O2/Os/nonrecovering sanitizer evidence, source pins and independent review are summarized in EVIDENCE.json. Default LeakSanitizer fails under ptrace in the recorded environment; same-binary detect_leaks=0 ASan/UBSan/float-cast passes are separate and NOT leak proof.

Authenticated isolated target Os/O2 diagnostics retain all three real APIs including double ingress and show unchanged4 double helpers, FMA-only1, then integrated0, with zero undefined symbols. Target layout remains144B/alignment4. This is NOT original or target instruction execution, current-firmware policy admission, full-sampler/case16 composition, code/RAM/stack/CPU/deadline/whole-product fit, audio/device/zero-dropout or release evidence. No SDK/library/toolchain content is included.

Full original preset graph, sample/oscillator/filter lifecycles, every parameter/modulation/effect, original sequencer/arpeggiator/clock/physical workflows, safe storage migration and exact native/resource/audio qualification remain separate unfinished product work. The historical approximate J6 proxy is not the endpoint. No original app/AU, assembly, sample/waveform/PCM, preset bank, compiled output or private report is included.
