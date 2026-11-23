# synths lane (Synth One and SuperJ6 finisher)

- Packed Synth One factory programs: `tools/pack_synthone_factory.py` writes a position-independent 61,208-byte pack (694 programs, 12 banks, 99,936 ordered writes; 15 flash sectors) and `firmware/src/dsp/s1pack/` reads it in place with no heap or static RAM; `synthone-pack` checks every name and word bit for bit. Not linked into the app image yet (reader 1,428 B at -Oz).
