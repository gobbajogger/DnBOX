# Factory sample starter kit: capacity and rollback feasibility

Scope: offline geometry and migration experiment only. Does not authorize
a firmware release, change defaults or reclaim any existing device data.
The proposed preloaded set is one BREAK loop and the complete authentic
909 kit. This is four out of five BREAK loops optional, not 80% of all
sample voices. The comparison pin is published 73749f55d17a.

| Zone | ADPCM payload | Role |
| --- | ---: | --- |
| BR1 1 | 17,400 B | Starter |
| BR1 2 | 20,894 B | Optional |
| BR1 3 | 21,431 B | Optional |
| BR2 1 | 23,092 B | Optional |
| BR2 2 | 26,457 B | Optional |

91,874 B downloadable original ADPCM payload. Existing-format full
Slot 1 and Slot 2 images are 60,237 and 50,061 B (15 and 13 sectors).
The starter-only image is 17,912 B (5 sectors). On a blank board that
means 23 fewer occupied 4 KiB sectors: 40 KiB in Slot 1 and 52 KiB in
Slot 2. These are unoccupied sectors within existing slots, not
permission for a different allocator to overwrite them. APP, FDATA,
Slot 3 and active user-data entitlement change by zero.

Pinned published geometry (end-exclusive, byte addresses):
- Slot 1: C2000–D6000 (80 KiB).
- Slot 2: D6000–E5000 (60 KiB).
- Slot 3: B3000–BB000 (32 KiB), unchanged.
- Three first-sector shadows: B0000, B1000, B2000, unchanged.
- E5000–F4000 remains the current S1PK FDATA, not empty capacity.

The 909 bass drum, snare, toms, rim and clap are already synthesized.
Its authentic sampled hats share 10,125 B; crash is 21,650 B and ride
23,500 B. Moving crash/ride could omit 45,150 B but is a separate
non-parity experiment, not part of this recommendation. Later FDATA
909 trials are different layouts, not additive with this accounting.

## First user-facing testpack

1. On a blank fixture, installer offers starter and both full original
   banks. The optional audio is downloadable, but re-expansion restores
   the exact full original slot images, pad order, IDs and MIDI keys,
   not optional-only renumbered groups.
2. A legacy project with absent content must preserve its settings and
   tell the user which exact pack is missing. Current content fallback
   can select loop zero. Fix that first: unresolved lanes must never
   silently play a different sound. A pre-change exact backup and
   stable content IDs or exact legacy manifest are required; no guessing.
3. Before any destructive write, create a new persistent atomic backup of
   all affected sectors, then reopen and verify the board/layout, sector
   boundaries and content hashes. Restore only the named ranges and
   reread them. The three first-sector shadows do not protect a full bank.
4. Rehearse starter-to-full-to-restore on the fixture with modeled missing
   ACK/power interruption and a previous populated Slot 3. Reads and
   mutations fail closed on failed backup verification or incompatible
   board. Do not overwrite APP, companion, Slot 3 or user assets.

Offline reversal smoke on deliberately blank factory fixture: expansion
changes 25 sectors because existing starter sectors also change. A model
tested 350 sampled torn erase/program prefixes, preserved an independent
synthetic Slot 3, restored the exact input slot bytes and rejected a
truncated backup. This is model behavior, not installed firmware proof.

Original full Slot 1 SHA-256: 77ae0a208740797b35f5a92164980e8a48b1529e5c003ff9e706481719500cf8
Original full Slot 2 SHA-256: 5c87d668124fff63d092acb0493c94515a9010463c148c0fc9bf522bea0ee947
Starter SHA-256: d98364a502bef8095eae56bb9142229d280c5061fb9ab5f46c709d93b3dcb139

No sample payloads are included. Verify original redistribution terms
before producing downloadable artifacts.
