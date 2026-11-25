# Typed original Synth One ARP RATE and CLOCK FACTOR, host only

Apply `s1-arp-clock-typed.patch`, then `original-clock-law.patch`. The first starts at the exact all-typed-controls
assembly followed by the identical A4 receiver/UI patches. Replay pins both
checkpoints and exact changed-file pre/postimages; run from this directory:
`sh replay.sh ../controls-init-only/source-mod-wheels`.

Private S1 ARP clock owner bytes 7–9 only. Little-endian integer BPM
1–200 and factor code 1–19; all-zero keeps legacy timing and does not enable
the independent clock. Interrupted/malformed scalar pairs reject at stage
validation. Physical detents use integer BPM; this is not a lossless codec
for the continuous original app Float32 knob. A4 byte 6 and SJ6-reserved
10–11 untouched, and no changes to base wire/version/lock fields. Uses
existing generation-bound Sound carrier and same owner apply/undo/load path
as A4, no ad-hoc physical or storage writes.

Physical SELECT and ARP PARAM K1/K2 via actual input and audio renderer,
both owners independent. 738 prior panel/event/clock checks preserved:
source note timing, audible output, handback after actual running transport
and held STOP, release/no hanging keys, sub-100 gate fallback and the same
STEPS/INTERVAL/GATE grid. Original panel-host rollback PCM baseline is in
the preceding s1-arp-clock-panel/app evidence packets.

415 additional checks cover old project no new page/writes, valid typed
row and byte-purity, physical HOME+REC/PLAY Undo/Redo, unchanged peer, malformed
rate/factor rejection, real save/staged project reload, then real played
and audible 137/207-sample onset. Turning both rates/factors on a recovered
previous generation is refused; read-back keeps the older exact selections
and save refuses without backing-store writes.

Host-only; not a native build, released feature, new bank/mode or storage
activation. Keep existing release gate, independent stock transport and BLE.
