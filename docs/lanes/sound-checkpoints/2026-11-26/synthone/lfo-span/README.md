# Synth One hoisted LFO phase increment, H16 donor
On the H16 product control map L1R and L2R are unsmoothed, and immutable during
an owned render span. Move the original binary32 rate/sample-rate division
outside the sample loop under the active CSM/external-sequencer combination.
Leave other modes' per-sample division, all phase updates, external arp, and
the order of downstream operations unchanged.
Apply the hash-guarded three-file patch to the H16 donor from the recovery
bundle. Build with the existing guarded H16 native link recipe. Both guarded
links complete, using 32 more executable bytes and the same usable pool.
The arithmetic model covered all four shapes with changing settings at
span boundaries. This donor predates the latest exact host effects and must
not be presented as a combined build or a release.

## Native outcome: reject
Both exact matching populated scenes fail with arp on. Elite peak timing
15558 → 15404 µs but underruns 39 → 150. Pro peak 15793 → 15409 µs but
underruns 53 → 134. Input note scenes, active project sectors and reserved owner
bytes match, no NOR writes and no skipped cycles. These are measured samples,
not a guess about why the peak and underrun totals diverge. Do not adopt this
change. The next performance arm must attribute onset/coverage as well as
dropout count and maximum; numerical workload models alone are insufficient.
