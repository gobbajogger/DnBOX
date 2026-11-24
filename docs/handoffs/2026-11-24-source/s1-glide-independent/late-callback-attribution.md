# Late callback attribution

## Native observer and qualification

The exact previous Elite prefix image and pinned runner have a paired TPROF receipt. The runner's `FM1_TPROF=PATH,FROM,TO` records weighted PC totals and cache misses across a time window. `FM1_BENCH_TRACE` and `FM1_BENCH_PCS` are bench modes. The detailed DnBOX observer requires `eng_prof` and `eng_prof_frames`, absent in the native production ELF, as is the debug-only `x0x_worst` record. The image retains an accumulated worst-block counter, terminal job estimates, and code symbols for the IRQ, render and job routines. Those values alone cannot give phase timing for the actual worst half.

The archived TPROF pairing uses the original firmware bytes, not guest instrumentation. In stock, timing moved from 10,867 to 10,843 µs and PCM changed. In Synth One + Miami, PCM, completed halves and all 134 XRUNs agreed, while maximum time moved from 15,714 to 15,716 µs. Both comparisons classify TPROF as attribution-only. The weighted profile was over 4.8–7.9 seconds. No independently qualified source identifies the late callback's per-stage times. See `trace-stock-4800-7900.json`, `trace-miami-4800-7900.json`, and `phase-profile.txt`.

## Host fallback

The controlled fixture archives tree `b1eaf004c2ac01670b11ee7dbf3ad9e5620fca51` and applies the prefix-glide patch only. It loads the same serialized program 63 Synth One + Miami project and checks all 2,712 Sound bytes against the stored Sound at boot. The host drives a reproducible dense sequence with both drum kits, both bass parts, and alternating live chords, with the shared arp enabled. Each of 384 halves calls the real renderer twice at 256 frames. It consumed 144 MIDI inputs, exercised all drum voices, and observed 30 arp step transitions in each run. The host uses serial execution. Its wall-time ratios and musical workload do not reproduce native DMA, UI interrupt cadence, or two-core stage ownership.

The independent control compiles with `X0X_ENGINE_PROFILE=0`. Three profiled reruns and ten alternating runs for the candidate below matched its full PCM and MIDI output byte for byte. Across the three ordinary profile runs, Synth One DSP took 46.3–47.3% of total render time, 808 took 25.5–25.8%, 909 took 12.1–12.3%, and Miami rendering took 9.0–9.7%. Synth One itself remained 43.8–49.6% among each run's 39 slowest host halves. This host workload's 808 share differs from the prior native scene, which attributed the largest core-0 engine shares to 909 and Synth One. Both point to actual synthesis and percussion rendering as the places to measure next.

Both parts ran a median of eight preparation calls per half. Their entire measured preparation cost was 0.14–0.17% of host render time, so removing only duplicated prepares cannot remove even that full share. The Synth One runtime wrapper, less time spent in the real Synth One DSP, accounted for 0.44–0.49%. It includes the shared arp and is a gross measurement. The linker wrapper for the per-sample arp predicate did not intercept its same-translation-unit call. Its zero count is uninformative.

## Exact arithmetic probe

In the measured Synth One stage, tested moving delay tap geometry and feedback multiplication to frames where the existing delay line consumes them. The shadow translation unit uses identical expressions and sample order. The full rate path still evaluates them every frame. Configurations that read those values for high-frequency envelope regeneration retain their original computation schedule. No controls, rate, storage, or sound defaults were edited in the saved firmware source.

The existing independent DSP oracle passed 36,900 tap-read and 34,689 feedback checks and compared equal PCM, histories and arena bytes over 2,400 spans and eight programs. Five alternating host pairs maintained exact whole-engine PCM and MIDI. Per-half median Synth One DSP deltas ranged from a 1.2% improvement to a 6.6% slowdown. The probe shows no repeatable gain and is rejected for integration. Source-level duplication alone does not establish that the compiled production image pays for it.

The result narrows the diagnosis to the 909 and Synth One render spans without a measured enough-to-close target for the native deadline. A useful further trace would need actual IRQ and job timings on the unchanged strict image with independent PCM, event and timing parity. This pass used no native slot. It did not address the queued effect-tail comparisons.

## Reproduce

Use a fresh writable scratch directory and check the focused storage reservation there first. The input repo must resolve to the verified tree. The scripts use the authorized local project and S1 pack and retain raw output only in the supplied scratch.

```sh
python3 "$ROOT/dnbox-runner-storage.py" --profile focused --tmp-root "$TMPDIR" --evidence-root "$ROOT/s1-glide-independent"
sh "$ROOT/s1-glide-independent/run-late-host-profile.sh" "$ROOT" "$SOURCE_REPO" "$SCRATCH"
sh "$ROOT/s1-glide-independent/run-late-delay-demand.sh" "$ROOT" "$SCRATCH"
python3 "$ROOT/s1-glide-independent/summarize-late-host-profile.py" "$SCRATCH"
```

Compact receipts are `late-host-stage-results.json` and `late-delay-demand-state.log`.
