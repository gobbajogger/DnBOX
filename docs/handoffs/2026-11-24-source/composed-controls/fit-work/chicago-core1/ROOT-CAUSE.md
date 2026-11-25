# Wheel/noise full-controls native late callback
Exact guarded Elite ELF `7af10a2519e1910dc28e189e96c684d88c5cd8e9afec4d441d3b9a807c47293c`. Passive emulator profiling and DMA trace only, no modified source/binary. All 922 physical scripted events and the stock-adjusted virtual seed are identical to the strict no-skip comparator. Profile runs repeat the exact complete PCM SHA256 `739af3712444c50a35d55d533a213c99c5fe5c6a0acc41d0757ea76488088d30`; final flash also matches. Weighted percentages below are per-core emulator PC weights in clipped windows; not direct cycle accounting of full callback or hardware timing.

Eight-second uninstrumented PCM contains repeated nonzero same-half indices 394/426/582, matching the exact total of 3 late callbacks. Audio scheduler trace gives actual index582 release 7,519.158623 ms and preceding release 7,507.553274 ms; this is the deadline window of the final repeated half. No new scripted controls arrive 7,450–7,545 ms. Compare the preceding adjacent nonrepeated active window 7,484.342–7,495.948 ms; use clipped profiles 7485–7495 and 7508–7519 respectively. 7497–7505 is a foreground/USB gap, not a useful active render comparator.

| PC weight | Adjacent on-time | Late |
|---|---:|---:|
| Core0 909 render + biquad | 25.59% | below profile top 32 |
| Core1 909 render + biquad | 22.83% | 36.84% |
| Core0 stage_wait + busy + sync | 14.33% | 34.30% |
| Core0 clock read (separate public helper) | 2.28% | 5.23% |
| Core1 BLE image XIP | 25.73% | 20.87% |
| Core0 Chicago voice span | 10.75% | 8.25% |

The late window does not show a larger peak in BLE or Chicago intrinsic voice loop. It shows assignment imbalance: the staged renderer at this placement sends the heavy 909 to the worker for both halves, and the first core predominantly waits for stages. The nearby on-time callback has 909 work on each core across its stretches. This demonstrates why a tiny drum arithmetic optimization has not resolved the 1.5 ms gap. Weighted PC mix cannot distinguish every inter-stage span; causal proof is the carefully isolated scheduler source eligibility change, with unchanged DSP and stack limits, in sibling `chicago-core1`. That change allows the currently excluded single Chicago to share Core1 and lets planner redistribute critical-path work; it explicitly forbids concurrent Chicago instances across cores because the packed lookup's shared memo is not atomic. Exact offset0 improves 12,419→10,815 µs Elite and 12,385→10,812 µs Pro, both 3→0 xruns. Offset1 is under hardware deadline, not under the tighter target. Further shift and state qualification pending; no ship authorization/claim.
