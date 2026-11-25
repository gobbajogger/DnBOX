# Immutable-data space check and region-local rollback witness

The stock SMK-37 flash map already has a 61,440-byte FDATA window
[0xE5000,0xF4000) and a 32,768-byte third user sample slot
[0xB3000,0xBB000). Neither is guaranteed empty. Both exact published
73749f55 packages explicitly contain type-103 S1PK, with data through
byte 61,208. The Init-only source omits the payload, not an erase.
XDATA is a protected asset region, and the app's own BLE area is not an
unused input region. Do not infer real-device occupancy from any archive.

The primary candidate is all three unchanged 909 streams in FDATA,
55,307 bytes including a guard header. This retains all BREAK slots and
uses the original physical boundaries. The alternate two-stream Slot 3
calculation 31,807 bytes has only 961 bytes left and cannot displace user
assets. Neither calculation authorizes a device write. See the native
relocation packet for actual links, cold guards and separate timing.

inspect_flash.py only reads a complete local 1 MiB snapshot. It scans
every candidate byte (not just the magic), fails closed on occupied or
unknown data, and explicitly does not authorize a write. Its tests use
synthetic arrays only; no real unit is touched. published-records.log is
a read-only metadata check of exact test archive package IDs.

replay-loader.sh extracts the existing C ldr_region/ldr_put bodies from
the exact current firmware loader and tests them against a synthetic
NOR adapter. It covers all 255 erase/program interruption positions,
both restore directions, partial writes, exact readback, idempotent
retry and adjacent-region protection. Malformed length/CRC checks
refuse before writes. These are region-local tests assuming the
originally selected staging bytes are still available; they do not
test interrupted real USB, bootloader record lifetime, full app-region
transition, last coordinated commit, or actual device recovery. An
installer must have a durable verified matching prior app + whole
FDATA before any write, and a reachable readback/restore route even
if the new app fails cold validation. Current normal package validation
expects S1PK. Do not bypass it. No packaging or live installer change.

The separate `replay-paired-loader.sh` pins the exact published loader,
OTA source, and loader fixture. For both Elite and Pro it replays 15
forward and 15 restore interruption scenarios on synthetic NOR and
restores the exact original published APP and full FDATA. It verifies
that an APP-only update leaves FDATA intact and that a partial FDATA
program is retried from the same staged bytes. The new APP and FDATA in
this test are synthetic mutations, not an installable candidate. This
does not establish physical USB recovery or adoption of a new package.
