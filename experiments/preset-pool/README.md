# Shared preset and BREAK pool — experimental, not enabled

DNL2 stores exact ordered binary32 parameter words, stable eight-byte preset IDs and names in independently installable banks. The 46-byte little-endian header points to a bank name, 32-word delta-coded global dictionary blocks, per-field palettes, shared ordered layouts and indexed program records. The offline encoder accepts already mapped engine-native field IDs. It preserves repeated ordered writes and does not quantize. The current schemas are Chicago, Miami, Synth One and SuperJ6; drum and BREAK control schemas remain to be mapped. The website must preserve persistent IDs across filtering and editing, and detect any identity collision before installation.

DBD3 uses the existing 120 KiB slot-2 range only: 28 data sectors and two alternating metadata sectors. BREAK data occupies a reserved contiguous prefix; banks can occupy mapped sectors outside it. Bank replacement and removal require a caller authorization callback, and sample reservation and release require a verified sample-extent callback. No physical flash binding is supplied. Do not enable formatting, widen flash permissions or change the updater based on this experiment.

Run from this directory:
```sh
python3 test_codec.py
python3 test_pool.py
```
The tests use synthetic records and simulated NOR, without any private factory data. The Python encoder is an offline reference, not yet a browser compiler. The wire dispatcher is separate from production USB ingress.

Before product admission: implement actual saved-project reference checks, persistent ID migration, a browser DNL2 compiler and USB maintenance session, real FSMP reservation/recovery, installer preservation and migration from S1PK/legacy slot2. Stage decoding outside the audio callback. Prove Elite/Pro fit and timing at the current source, then exercise sample and preset recall and interrupted updates. The prototype is deliberately outside the production build. No device writes or firmware release.
