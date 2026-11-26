# Warm SuperJ6 manual delay expansion
A real chronology repair, not a broader A/B fidelity or fit claim. Stock only
carries old history forward with an active modulated Delay Time route on stride
growth. In an equivalent manual/clock change, it clears all complete cells and
pending audio and jumps to the new tap. This one-file patch reuses the exact
existing routed in-place coarsen for valid warm nonrouted expansions with at
least one complete coarse bin and a representable old physical delay. The
newest complete cell anchors sample age. The pending store remains a separate
undivided sum and retains its exact sample count. Physical delay is conserved
when rescaled, then transitions using the actual 1,250 8-frame control ticks.

No additional history, heap, struct fields or DSP-loop source. For cold and
silent starts, routed, same-stride, decreasing, cold, beyond rate range, or old tap
less than one new stride, this does not introduce a new policy (ordinary same-
stride still ramps; unsupported reconfigurations retain their existing
fallback). No claimed improved alias rejection, insertion/cross-channel changes,
or ultra-long genuine Build9 audio; comparator is exact stock compact.

The focused host witness compiles unmodified include independently against
the proposed include, serves 44.1/48k, signed stereo, three feedbacks incl
maximum, manual close-boundary growth, mode and tempo growth, cold all 128
raw values at four tempos with frees and sync and route guards, same stride,
hop to decreasing stride, very early, minimum-frame, unrepresentable and
invalid transitions. For all 24 qualifying examples compare actual in-place
history words independently to newest-first chronological double-array sums
and truncation, including wrap. These all retain nonzero actual wet first800
during silent ingress vs baseline exactly zero; all hit exact ramp target.
4,114 unchanged guard states verified. No deliberate ping-pong adaptation or
broad validation. The format retains its limited bandwidth and can't recreate
detail or old buckets on shrink. An instantaneous coarser filter at transition
can still change wet color; no no-click wide-bank claim. Worst existing
coarsen walks at most 1024 input cells (two i16 words each) in one edit; that
bounded work is new for manual edits, so integration should qualify a warm
slot edit under actual polyphony stress, rather than claiming no xrun from
the unchanged steady render loop alone.

Actual isolated JieLi PI32v2 target translation unit at Os: 2172 ->2216 text
(+44), 36 rodata both; process symbol 738 both. No linked or runtime gate.
Run `for x in base new; do cc -O3 -ffp-contract=off -shared -fPIC -I"$x" probe.c -lm -o "$x.so"; done`,
then `OPENBLAS_NUM_THREADS=1 python test.py`. The appropriate source patch
applies against firmware/src/dsp/juno6_delay_native.inc only; keep other edits
independent when integrating. No external audio, binaries or other assets.

Independent warm original-source comparator: the existing Build9 behavioral
C port (verified in twelve prior isolated leaf cases) actually retains its
3-second stored audio when setting a second delay tap from .248 to .496 s.
In six new 44.1/48k and feedback 0/64/127 cases it emits nonzero wet in both
channels through the first800 samples of silence after the edit. Patched
compact output also retains both but is *not* raw audio equivalent (compact
uses a held/averaged history and ramps delay; the tested original core
repositions its tap immediately; hidden original wrapper control cadence not
qualified here). All three feedbacks in compact invert the signed stereo
stimulus and output exactly, with mismatched input partitions. Compile
`cc -O3 -ffp-contract=off -shared -fPIC original.c -lm -o original.so`,
then run `OPENBLAS_NUM_THREADS=1 python source.py`.
