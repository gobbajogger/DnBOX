# Exact compact SuperJ6 voice reducer measurement

The instrumentation records the already computed signed Float32 layer sum
immediately before stereo routing, without changing the reducer or the
audio. The compact source on the pinned DnBOX donor is

```c
float x = mix[l] * (p->gain / 6.0f);
left[k]  += x * (.5f - .5f * p->pan);
right[k] += x * (.5f + .5f * p->pan);
```

With one factory Sine layer, one A4 at MIDI velocity 87, unity layer gain,
the other oscillator muted, and neutral pan, both legs arrive at the
reducer with RMS .195261. The current original receiver's raw neutral
Sine RMS was .195687 under its separately matched cue (within 0.019 dB).
The compact reducer produces .016272 RMS per side, exactly −21.58 dB
relative to its own pre-reducer sample. This identifies a compact firmware
level discrepancy **before effects**. It is separate from the unobserved
loaded-AU signal stage in the comparison model.

Removing the reducer globally is not admissible: six distinct test notes
with oscillator steps forced coherent for peak worst-case exercise have
pre-reducer RMS 1.171567 (and absolute peak above unity; compare reported
legacy peak .138137 × 12). The unchanged mixer maintains its bounds.
Noncenter panning remains asymmetric and exact. A shipping correction
must retain intentional polyphonic headroom and pan behavior.

Reproduce with `bash verify.sh path/to/dnbx_core.c path/to/headers`.
`instrument.py` checks the exact DnBOX donor SHA. The matching compile
headers and donor are indexed in the adjacent `sj6-osc2-generated-amp`
receipt. This tests the standalone typed core, not a full loaded target,
not the downstream mix ceiling and not a release candidate.
