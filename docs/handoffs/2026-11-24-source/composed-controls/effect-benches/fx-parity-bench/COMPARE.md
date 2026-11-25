# Stereo synth FX comparator

Python 3 + NumPy, no audio service or device. Use the same deterministic
excerpt and initial synth/FX state in original and compact, and render each
once dry and once wet. The tool computes the Dry null, the full Wet null, and
the isolated FX null:
`(compact-wet - compact-dry) - (original-wet - original-dry)`. A wet-minus-dry
difference is measured as captured; it may include a real FX-path change to the
dry component and is not asserted to be an independently tapped wet bus.

```sh
python fx-parity-bench/compare.py \
  --original-dry ref-dry.raw --original-wet ref-wet.raw \
  --compact-dry new-dry.raw --compact-wet new-wet.raw \
  --raw-format f32le --rate 48000 --tail-start 2.0 --output result.json
```

Default PCM16 stereo WAV; raw is little-endian interleaved f32 (digital full
scale 1.0) or signed i16 (/32768). Per-input `--original-dry-format`, etc.,
can override the common format for mixed-format comparisons. All four rates
and complete stereo frame counts must match. No gain, resampling, cropping
to shortest file, polarity, phase, automatic latency search, or time warps.
`--compact-leading-samples N` explicitly skips N initial compact frames if
positive or -N original frames if negative, dropping non-overlap only.
`--window START:STOP` and `--tail START:STOP` are half-open frame windows
relative to this overlap; `--tail-start SECONDS` is an alternative from
aligned frame 0 rounded to the nearest frame and runs to overlap end.
No tail is silently selected; supply a known note-off/excitation-stop.

Null/error: RMS, dBFS, relative dB, per-channel RMS and peak. Stereo
Pearson normalizes only this descriptor, not the samples/null; absent,
near-silent or constant channels report undefined correlation. Band powers
use a fixed Hann window, single-sided FFT and stereo averaged power. An
undefined dB (including exact zero error) is JSON null, with explicit RMS
retained. Tail blocks track each wet-minus-dry RMS and level delta.
Silence is peak-based, configurable with `--silence-peak-dbfs`.

Report-only by default. Optional `--max-dry-null-dbfs`,
`--max-effect-null-dbfs` and `--max-tail-null-dbfs` compare linear null RMS
to the supplied dBFS-derived bounds. One-sided activity mismatch fails a
supplied check; both-silent is inconclusive, not an exact parity pass. A/B
status is JSON only; valid input returns 0 even on threshold failure, invalid
input returns 2. This is not a release gate. `--self-test` synthesizes its own
signals; it does not invoke either engine or contain private captures.
