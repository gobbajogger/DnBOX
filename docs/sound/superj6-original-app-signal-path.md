# SuperJ6: what the current original app actually wires

Current original ARM64 binary SHA256 `6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850`. The graph below comes from executing the compiled original constructor and observing reached typed connector arguments, plus inspecting its sampler code and rerunning its scoped receiver. It is not inferred from stereo output. The test stops at external Apple graph boundaries; it does not claim installed internal bus PCM, queried tap format/gain, or that a disconnected fixture is the loaded app.

## Voice and connection order

- The fixed oscillator fan-in is **Main 1, Main 2, Sub, Noise**, each an independently constructed sampler. The constructor builds that four-item array at `0x1002b8914…8938`, reaches AKMixer at `0x10047b944`, and requests each source bus 0 via the real `connect:toConnectionPoints:fromBus:format:` send at `0x1004775c4`. It reuses one requested stereo 44.1 kHz format. The observer does not claim the actual destination bus IDs, negotiated format, or Apple mixer rendered levels. Aux is a separately constructed/control-addressable sampler; it is not a fifth member of that fixed array. No blanket claim about alternate graph activation.
- At matched-input sampler taps, **AMP/voice gain is before the Normal IIR**, which maintains independent histories. The original left/right output is the exact sum of the voices observed at `0x1003b7d34` / `0x1003b7d40` or the Normal branch `0x1003b7938` / `0x1003b7d40`. The optional common startup ramp at `0x10037b358…37b3fc` ran only during the Raw cue's initial silence. It does not explain the warm level error.
- Original note/receiver tracking uses the actual envelope, cutoff, velocity, and pitch state, not a generic per-frame cutoff multiplier. The Normal source call at `0x10037fdbc` receives the per-tile physical cutoff; the direct panel receiver truncates standalone physical #36 values before subsequent tracking and modulation. At `0x1003b59c0…59d0` the reached source advances a per-voice envelope with an ordered double multiply-add and passes the new level into filter control. Destination model changes wait for the confirmed gain-zero rearm at the next callback-local tile. Supporting source/operand receipts are separate, not a claim that every installed LFO write was captured.

## Original downstream graph

```text
Oscillator mixer
  → Unison → High-pass → Tremolo
  → Crusher → Booster → AutoPan → Widen
  → Classic chorus → Booster → J60 chorus → Delay
       ├─ Rage/dry branch ────────────────────────────────┐
       └─ wet input level → wet high-pass → Room           │
                              → wet-only PeakLimiter       │
                              → wet output level ─────────┤
                                                           ↓
                                                        Summer
  → master booster → master attenuator → tracker → DC removal
```

Three first edges reached at `0x1002b89b8`, `0x1002b8a04`, and `0x1002b8a8c`. Nineteen downstream connector argument witnesses reach the compiled connector `0x10044bc3c`; the constructor starts its Tremolo downstream link at `0x100059e64`, reaches Delay `0x10005a0a8`, wet input `0x10005a0f8`, Rage constructor `0x10005a2ec`, summer `0x10005a328`, then post-sum links `0x10005a370`, `0x10005a3b4`, `0x10005a400`, `0x10005a44c`. These are connected nodes, not a claim that every effect is audible. Example: Delay Enable changes the dry/wet setting on its existing delay, not the graph; the wet Room branch uses a separate input level. The shared resonance to oscillator-mixer volume callback is observed as a separate property: Raw unity and Brainwave about .867. This is not a master-volume gain fit.

## Current DnBOX differences and matching order

1. Compact Sub and Noise are summed into Main 1 **before** Main 1's filter, rather than separately processed sampler streams. Main and Sub prototypes with independent state did match controlled original filter leaves; the all-source live renderer is still not wired. Do not use one shared nonlinear OTA model for those roles.
2. Compact `juno6_render_stereo` performs HP before `sj6_run_effects`; Unison is currently after chorus and Delay. The separate source-order candidate moves only Unison and HP into the proven sequence. It does not silently elect alternate effect modes, add gain, or treat compressor approximation as proof.
3. Our one-choice chorus, late crusher approximation, serial Room/limiter processing, and post effects layout differ from the original. In particular, the original limiter in this graph is **not a whole-bus limiter**. A correct comparison needs the same post-Delay drive at both branches and a separate wet limiter test. Do not just move the existing limiter or retune presets.
4. Verify matched note owners, matched sampler inputs, separate branch histories and callback-local control timing first. Then compare whole unnormalized signed output with original held/release cue, and measure the exact same enabled graph on Elite and Pro. The external app mixer values not observed by these tests stay unresolved, not substituted with fitted trim.

Earlier causal checkpoints: [Normal filter and AMP](checkpoints/2026-11-27-sj6-normal-first-divergence.md), [vibrato pause experiment](checkpoints/2026-11-27-sj6-vibrato-arm-no-adoption.md). Full parity and zero dropout are not claimed.


## Authenticated control routing

The compiled app runs LFO1, LFO2, then LFO3. For routed effects it clamps the normalized value and passes it through the same original public parameter dispatch as a direct edit. This does not mean the user can select every internal writer: the original public MOD picker is a serialized 71-row table, and selected rows are converted back to indices in that table.

| Original public effect destination | Zero-based picker row | Parameter ID |
|---|---:|---:|
| AutoPan depth / rate | 5 / 6 | 23 / 22 |
| Delay feedback / mix / time | 9 / 10 / 11 | 26 / 27 / 25 |
| Unison detune / stereo | 45 / 46 | 60 / 61 |

Delay enable, Unison enable/type, Classic Chorus and J60 are absent from this pinned constructor's public picker, even though several of their controls exist in the internal writer lookup. The Version path replaces matching existing chorus rows if present; it does not insert absent rows. J60 starts only when the combined Chorus mode is nonzero and the internal Version byte is 1. Crusher mix callback is #110; it should not be conflated with #109 Bit Rate.

The wet input high-pass and Room feedback use the reached original parameter transforms: normalized Room Size becomes `0.4+0.6*u` and Low Cut becomes `800*u^2.42`. Type II stops the wet limiter; a diagnostic fork has passed an 8192-frame chronological stimulus through the authenticated Apple high-pass leaf, an older reached Costello Room implementation, and independent dry/stopped-Rage and wet paths. This proves the branch construction and tests that candidate, not that the installed current active Room and Rage match. No whole-engine parity is inferred.

See [reached filter-envelope behavior](superj6-source-envelope-coefficients.md) for the source-backed envelope, recurrence, and clock.
