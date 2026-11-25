# Scoped host correction: Build 9 classic chorus OFF/I/II/Both

Use `sh perf-final-proof/sj6-classic-modes-oracle/replay.sh` and
`sh perf-final-proof/sj6-classic-modes-oracle/run-prior-red.sh`. Replay re-hashes
the authorized original AU; no original executable/media is in this packet.
The source checker verifies the public callback shares the same classic graph
for all three modes; J60 and private selector address 6 remain distinct.

The application submits the mode's frequency first (.513, .863, 1.726 Hz),
reads it, projects through inverse/Float32/forward, then reapplies normalized
Depth and Mix. Depth is .202 reprojected twice by .97; Mix .505 for I/II,
.555 for Both. OFF pauses active processing and does not reapply those
controls. Mode entry changes targets but not phase, delay history or ramp
duration. No new storage or mode selector is justified by this evidence.

Corrective finding: the original classic process writes mode-additive
feedback offsets first (left and right), but *overwrites* the same exact
delay feedback slots before calling render. What reaches the actual delay
write is binary32 raw feedback multiplied by the Float32 FMA
`1 + .05*sin(phase)`, with .2-sample right sine timing offset. This
supersedes the additive-offset finding in the earlier Classic I packet.
At Init raw feedback zero, no recirculation due to those earlier offsets.

Selected-graph HOST scope: 26,577 decoded stereo frames are bit exact against
the independent instruction-order oracle across two owner objects, active
three-control edits, mode transitions/ramp/history and dry OFF. Original app
executable/audio capture at arbitrary sample rate, J60 and armed active
SMK II/Both/cold persistence are not qualified by this packet. Existing I
compose needs `correct-classic-wrapper.patch` (just two core files) and fresh
real physical fixture. Do not ship or expose new options based on this layer.
