# Isolated size experiments

The accepted baseline remains `../src/dnbx_core.c`, SHA-256 `9cdcea0b90b691d05cd07306433b9b4cf42c023bf419a251d7ff436fa13487fa`. These variants are separate inputs for measured native size review. They do not silently replace the baseline or establish a whole-product fit.

## Physical validation table

`dnbx_core_bounds_table.c` replaces repeated physical-parameter checks with a fixed table containing each of the 23 float fields' individual lower/upper bounds. A compile-time offset assertion checks that the table corresponds to the physical parameter layout. Values are read through a four-byte `memcpy`; there is no aliasing assumption about a struct being a float array. The oscillator mode remains a separate checked integer enumeration.

Strict ordered comparisons against finite bounds reject NaN and infinities; the redundant exponent-bit check is removed from these bounded comparisons. The unbounded raw-value storage path retains its explicit finite-bit test. No incoming value is clamped, no physical parameter is reinterpreted as a normalized control, and all original units/ranges remain intact. **Strict floating-point semantics are required; do not compile with `-ffast-math`.**

The 12 worker tests pass. On the local host at `-Os`, text plus read-only data changed from 7,733 to 7,327 bytes, a 406-byte reduction. Host sizes are not native flash measurements. The native resource worker measures the same complete replacement closure independently.

## Explicit diagnostic-counter profile

`compact_profile/dnbx_core.c` combines the bounds table with a compile-time profile in its matching `dnbx_core.h`:

- `DNBX_DIAGNOSTIC_COUNTERS=1` is the default and retains the promised render counters.
- `DNBX_DIAGNOSTIC_COUNTERS=0` removes render counter branches and increments. `dnbx_diagnostic_counters_enabled()` returns zero for this profile.
- In the disabled profile, passing nonnull `work` to `dnbx_render`, including a zero-frame call, returns `DNBX_E_UNSUPPORTED` before state/output mutation. It does not silently ignore requested counters.
- Events and controls remain excluded from semantic counters in both profiles. Complete selected event/control timing belongs to the test-only harness, not extra production bookkeeping.

The C ABI structures and DSP state are unchanged. The capability query and error code are additive. The normal native replacement adapter passes null work; native LTO may already specialize those branches away, so a counter-off saving must be measured and is not assumed.

The compact profile compiles with either value using its own header first:

```sh
cc -I size_variants/compact_profile -I src -std=c11 -O2 \
  -ffp-contract=off -fno-fast-math -DX0X_HOST \
  -DDNBX_DIAGNOSTIC_COUNTERS=0 \
  size_variants/compact_profile/dnbx_core.c tests/test_core.c -lm \
  -o size_variants/compact_profile/test_compact
```

`compact_profile/EQUIVALENCE.json` records a three-way comparison among the baseline, the variant with counters on, and the variant with counters off. It checks 234 typed boundary/admission cases and exact rejection behavior, then compares audio and complete state after every irregular render call across two parts, 16,384 frames per part, and 324 calls per part. All audio and state bytes match; the same audio digest is recorded for each build. The disabled profile's nonnull-work error is also checked for state/output nonmutation.

Both profiles pass all 12 functional test groups: 424,596 assertions for diagnostics on, 424,591 for diagnostics off. The five-assertion difference is the explicit profile-aware counter test, not omitted audio/lifecycle coverage. These tests omit the large trace-export pass included in the baseline's 465,568-assertion receipt.

Frozen compact-profile hashes:

| File | SHA-256 |
| --- | --- |
| `compact_profile/dnbx_core.c` | `3d8f898428b297478abb35692de56952451caed38cc02db53e16a19a54a7408b` |
| `compact_profile/dnbx_core.h` | `86acaf17ee9c48dac151919098ab12de3acf457fab2bd9fdf57ac966f36c0962` |

The independent acceptance worker owns a separate frozen comparison and the native resource worker owns replacement flash measurements. Passing these equivalence checks alone does not satisfy native flash, full callback WCET, complete Synth One/SuperJ6 behavior, or listener approval gates.

## Measured native result

The actual reachable replacement closure at strict `-Oz` measures 553,420 bytes for Elite and 553,412 for Pro with either counter profile. This saves 152 bytes versus the frozen baseline replacement (553,572/553,564). There is no additional counter-off saving because normal native LTO already specializes the adapter's null-work calls.

The compact result still physically exceeds capacity by 748/740 bytes and falls short of the required 1,024-byte free-XIP floor by 1,772/1,764 bytes. Its state layout is unchanged. This is a measured reduction with an unresolved flash blocker, not a hardware-fit approval. Exact owner receipt paths are preserved in `../evidence/native_resource_summary.json`.
