# 909 RAM and held-REC ARP follow-up (lab)

Built on the qualified 332730b3 lab source and its six flags. The qualification branch deliberately does not choose a public build profile or enable a new installer release.

- Compact the 909 LT/MT/HT panel-only records without removing their actual oscillator state. Host layout saves 1,152 bytes. Host differential matched 9,160,980 bytes over 2,307 blocks across serial, helper and fallback paths; optimized and sanitizer variants both match exact.
- A pending ARP reset now respects all currently held local keys, not just the most recently pressed one. Added newest-first release regression and kept immediate USB-MIDI takeover and exact transport-grid tests. Focused host hook suite passed (private SJ6 reference bank absent; core SJ6 note, clock and AMP2 checks passed).
- Both native models link. Shared pool after reserve 21,832 B each. XIP free 852 B Elite, 864 B Pro. No timing claim for these binaries until their exact replay receipts are complete.
- Separate build-composition test-only correction asserts the actual fast Chicago provider and its narrowly scoped data-packing flag. All 21 cases pass.
