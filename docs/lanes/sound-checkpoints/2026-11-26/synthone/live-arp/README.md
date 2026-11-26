# Independent Synth One original/firmware arp events (host only)

The DnBOX arm executes the firmware C sequencer, held-key adapter and source-clock receiver independently; it does not read the upstream event tape. The final proof opens the actual 906-program factory pack, loads programs via s1_part_program, and admits loaded values into the optional factory helper.

Before the patch, the retained app takes existing project ARP and source-clock panel settings and has no product caller of engine_arp_from_s1. Cold owner without a block starts arp off. With explicitly mapped controls but default project clock, all eight focused live phrases diverge. The existing original source-clock receiver at exact authored BPM/detent independently matched 7/8. In Upside Arp, both clocks fire at frame 5,640, but original emits note 55 and old mapping emits 56: original truncates after notes-per-octave scaling; old mapping rounds 7.71 to 8 prematurely. Four of all seven fractional factory ARP rows audibly diverged on this cue.

The quarantined optional patch keeps the factory Float32 pitch values until after source NPO scaling/truncation. A successfully loaded non-INIT Synth One program with no saved ARP block or pending edit can start its authored pattern. Explicit stored ARP including OFF, typed project clock, and running project/external transport retain precedence. Factory fallback never writes the user's project. A project control edit takes ownership using the existing representation. Unsupported or unavailable clock, INIT, or invalid program cannot silently enter factory playback. The opt-in requires a real pack path, not the current INIT-only profile.

- Real-pack verification: 393/393 active programs matched original 22,396/22,396 event tuples, including six NPO classes; 14 active reference tapes checked against the unaltered full original renderer.
- All 16 focused factory-packed independent event tapes gave the exact same release-phase donor wet PCM as previous shared-tape verification, including two arp-off guards. No gain or alignment. Host chunk boundaries were held fixed; the real app's renderer segmentation and transport transitions were not tested here.
- Actual runtime and engine files passed host syntax checking. Loaded helper rejects saved owner, pending edit, INIT and invalid status while preserving config bytes. No target build, fit, timing or hardware checks. Custom NPO follows the pin's authenticated common host project tuning.
- Primary records: bank-event-packed-results.json and final-audio-results.json.

Reproduce from workspace root and the exact restored inputs:
    bash s1-live-arp-check/build.sh
    bash s1-live-arp-check/build-patched.sh
    S1_TEST_REAL_PACK=1 python s1-live-arp-check/run-bank.py
    python s1-live-arp-check/run.py 29 34 42 48 62 113 154 271 416 421 487 496 738 740 751 880

Generated source trees, full event dumps and PCM remain private scratch and are not included.
