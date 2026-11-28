# Six Synth One voices with both kits: post-render timing and firmware memory

Start from `six-loaded-preferred-start.md`. This is the Elite dry Synth One
diagnostic with six MIDI-held voices and 132 actual 909 plus 132 actual 808 hits
in the held interval. It is not the complete scene and does not qualify wet
Synth One. Do not install these diagnostics.

## Timing attribution (separate instrumented build)

`post-join-profile.patch` and `read-post-join.py` use the same fixture and
strict emulator, zero skipped cycles. Held-window means per 512 generated
frames, not worst-case latency:

| Phase | µs |
|---|---:|
| After joining the jobs: cleanup and send shaping | 51.17 |
| Bus FX and master, including inter-core waits | 827.63 |
| Block setup | 200.28 |
| Sequencer work before/after subspans | 203.75 |
| Deferred master | 5.80 |
| Final output conversion | 61.22 |
| Scope and meters | 27.68 |
| Whole engine, including the voice/drum jobs | 13,065.53 |

Do not add the whole-engine row to the others: it includes them. Even an
unrealistic zero-cost bus FX/master wouldn't be enough to close the measured
3.50 ms peak gap. The instrumented build is for attribution only
(14,446.96 µs peak, 151 XRUNs), not an optimization measurement.

## Independent optimizations

- `break-derived-offsets.patch`: derives each of the 33 BREAK slice boundaries
  rather than storing four copies. Native Elite and Pro each recover 528 B of
  pool (26,492→27,020 B after reserve at otherwise identical settings), at a
  32 B app flash cost. No changes to ordinary RAM or stored bank format.
  Standalone adaptive oracle checks passed (2,214,169), BREAK 22.05 kHz and
  44.1 kHz each passed 89/89, and 7,040,512 compared host PCM frames matched
  bit for bit. Native BREAK-active performance is not qualified.
- `crc-shared.patch`: removes the duplicate nibble CRC table/body from native
  app objects while leaving the loader and standalone host units independent.
  64 B app XIP recovered on each model (Elite 2,948→3,012 B, Pro 2,956→3,020 B
  on the loaded baseline); no pool delta. Existing boot-screen, FITTAB, and
  XDATA host checks passed. This does not touch the synth or drum audio hot path.
