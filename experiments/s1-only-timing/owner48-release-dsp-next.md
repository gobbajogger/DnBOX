# Where the remaining deadline is spent

The [paired 48/49-dropout source checkpoint](owner48-release-dsp-result.md)
is the starting point. Keep its four-patch source composition. This page
records attribution and a rejected extra optimization, not a new winner.

A [separate instrumented Elite run on that exact composition](owner48-release-dsp-barriers-elite.json)
still traces its slowest held block to generated frame 129,024. Core 0 uses
2,495.04 µs for the 808 prefix and 7,383.50 µs active Synth One time
(8,176.67 minus 793.17 waiting). Core 1 uses 5,256.42 µs for the 909 plus
4,850.75 µs for the rest of the 808. The serial tail adds 990.58 µs.
That is roughly 10 ms of active work on each core before the final stage.
Further patch work should reduce this compute, not shift whole jobs blindly.

The same generated release frame 171,520 now measures 12,724.50 µs
instrumented, down from 15,178.62 µs in the [baseline probe](owner48-release-barriers-elite.json).
The Synth One wait on the 808 falls from 5,670.04 µs to 592.08 µs.
These are matched generated-frame observations, not the uninstrumented
release outcome; the probe perturbs timing. All uninstrumented results
and audio fingerprints remain in the paired checkpoint.

Rejected: [matched-pan product sharing](owner48-808-matched-pan-product.patch)
on top of that complete composition. [Canonical Elite](owner48-release-dsp-plus-pan-elite.json)
still has 48 dropouts, but the held max rises from 12,751.21 to 12,811.54 µs,
physical held misses rise from 34 to 36, and free app flash falls from
1,376 to 1,296 B. It was not repeated on Pro or adopted; its isolated
808 improvement did not carry over.
