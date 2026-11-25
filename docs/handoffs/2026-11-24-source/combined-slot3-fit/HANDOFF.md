# Additional APP fit capacity, guarded build only

This source/link increment follows the owner's order: add authentic
controls, make combined firmware fit, then test the complete result.
This is the existing four-way joined AutoPan/VOICE/Gate/RANDOM2/
Delay/Filter Velocity candidate, not yet all requested controls. No
behavior, emulator, package or physical device step was run here.

Both existing Chicago A/E compressed lookup payload bodies relocate
byte-for-byte out of APP into the existing, previously discussed 32 KiB
third sample slot as a read-only diagnostic companion. No tone or
control is removed, no first/second sample slot or shadow touched,
no preset bank changes. No stock/device migration asserted. Native
actual links:

- Elite APP headroom 30,880 B and Pro 30,892 B.
- Both POOL 520 B above existing 8 KiB reserve.
- The proposed companion occupies 24,728 of 32,768 B in Slot 3.
- FDATA remains guarded separately at 60,944 of 61,440 B.

Exact final linker-body section sizes, addresses, compiled ADPCM
asset equality, application digests and companion CRCs are enforced by
the link. Both data records must match both the board and exact APP
before it initializes audio. With X0X_COLD_SLOT3_LAB only, the application
refuses USB sample begin/write/end/erase for slot 3 and suppresses
automatic legacy shadow restore to that span; the normal profile's
sample behavior and geometry are unchanged. This is only a never-install
development linker artifact. The old user's contents are not assumed
empty: stock adoption would need an independently verified preservation
and rollback path, and all later release gates. No such packaging exists.

Replay from the physical fit-first donor (five-field Params layout
already installed). Apply slot3-fit-guard.patch at fuzz=0 and run the
vendor linker via link-slot3.py for each of Elite and Pro. It uses the
same standard data/code boundaries, explicitly no new physical writes.
