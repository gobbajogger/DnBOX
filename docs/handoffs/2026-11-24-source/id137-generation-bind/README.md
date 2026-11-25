# ID137 exact joined virtual-store same-version tuple coherence

Predecessor: source-mod-wheels full manifest b71f366f… + exact joined
host witness perf-final-proof/sj6-id137-joined-carriage/combined-host.patch.
Baseline replay reproduces 53 Sound + 22 lock accepted (1,526 UBSan checks).
This packet consumes neither row bytes nor additional object bytes. Do not
apply to a shipping format or treat this as migration authorization.

Native test-only STREAM16 first object's +4..11 use a FNV-1a64 tuple binding
instead of its fixed/inferable raw length and standalone 32-bit stream CRC.
The format, packed length, decode length and total object limits remain.
Digest domains STREAM16, the exact persisted part-off, actual settings
except Bluetooth device bit 0 (intentionally independently saved), exact
Sound and all decoded raw patterns, song/arrangement and motion/note locks.
No sidecar offset, portable CRC/header, or standard project revision
changed. Existing per-object structural validators still run; malformed or
newer object format is not eligible for fallback. FNV is for accidental
coherence, not cryptographic authentication, hostile data or collision
resistance. Candidate numbering/header was never assigned for product.

Latest tuple gets first priority. Recovery can probe at most 63 A/B
combinations after a recognized typed mismatch/decode failure, without a
second full project/stream buffer. The storage implementation actually
accepts PLAT_STORE_PREVIOUS for SET, SOUND and all stream objects, not
just the prior stream caller. Recovered exact tuple is **read-only**:
normal save and independent device BT write are blocked, preventing repair
from accidentally erasing the only last coherent object. Repeated checked
latest coherent mount can clear the flag; no automatic repair or active
migration implemented. Unsupported or no-match load leaves safe defaults
and blocks both ordinary/BT save; neither live nor staged parser should
apply the torn candidate. Logical equality is sufficient when an unchanged
object was skipped by the actual writer. Repeated A/B probing may incur boot
CPU/flash latency. Save/recovery must be qualified on real-sector cut/fault
simulation before any product decision; this is the virtual store path only.

Retained 1,597 UBSan assertions. Dense fixture seeds 24 patterns using
valid varying bass events and consumes three stream objects; actual writer
can commit SET, SOUND, three used segments and an unused exact tail.
Inject reject at each write callback in order. 0–4 commits recover exact
old values, palette and patterns; after the fifth commit the exact new
tuple is complete, whether or not trailing unused segment is touched.
No write on load, or when attempting to save recovered generations.
A separate deliberate same-revision torn Sound transplant with no
coherent prior rejects -9 and does zero writes, as does unknown format.
Independent Bluetooth-only persistence after complete commit works.
Original untyped opaque legacy and all physical ID137 fixture checks
remain green. Small predecessor case passed 1,571 independent checks
before dense case; retained receipt is informational.

Production blockers: profile is test-only; native latency/fit, exact
flash interruption model including recovery maintenance, future-schema
versioning, multi-update generations and explicit recover+repair policy
remain unqualified. One earlier coherent digest can be absent because
A/B retention is limited; then refusal is safe, not automatic recovery.
No firmware/device/Git/package/publication changes.

Replay: sh replay.sh /exact/controls-init-only/source-mod-wheels. Requires
small disposable scratch and the same generated host headers as joined
proof. No authored or mutable shared source inputs touched.
