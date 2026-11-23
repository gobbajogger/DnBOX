# synths lane (Synth One and SuperJ6 finisher)

- Packed Synth One factory programs: `tools/pack_synthone_factory.py` writes a position-independent 61,208-byte pack (694 programs, 12 banks, 99,936 ordered writes; 15 flash sectors) and `firmware/src/dsp/s1pack/` reads it in place with no heap or static RAM; `synthone-pack` checks every name and word bit for bit. Not linked into the app image yet (reader 1,428 B at -Oz).
- SuperJ6 factory presets: `superj6-presets` loads all 640 programs in 12 banks from the private export (`SJ6_PRESETS`, default the box copy; never committed) through the C bank reader and prints `presets=640 unmapped=0`; the two legacy names map to the limiter stage inputs. It prints SKIP without the export.
- Host tests aligned with merged behavior: `smc-buttons` expects the `NEXT --` cancel text from ccf451a6, and `host-setup` expects the drum generator to run on every entry (e12a63c6).
- Base-red fix: align padcal-wiring, owner-copy and clip-chord-display host tests with merged ui.c/gfx.c (ccf451a6, 837530d9).
