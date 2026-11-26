# Correction: public Crusher dispatch is not internal setter ordering

Supersedes public labels in sj6-fx-namespace, sj6-crusher-receiver, converter
and #109 private carrier/live drafts. The tested phys/Float32 setter math is
unchanged but its public attribution was wrong. Do not adopt #109 cadence.

| Namespace | v0.999.8 Build9 route |
|---|---|
| registered #107 neoncrushEnable | ivar +bc8; edge .2737c8→.f1440; wet 0/.3 + Booster 1/1.3 |
| registered #108 neoncrushAmount | ivar +bd8; edge .27387c→.f1500; Apple #3=100×.0606×pow(raw,1.23), Apple #4=0 |
| registered #109 neoncrushBitRate (Lo-Fi) | ivar +be0; .273898 metadata only; no physical DSP setter |
| internal/unregistered #110 mix | ivar +bd0; edge .273970→.f1598; Apple #15=100×pow(raw,1.5); not public |

Machine checks tie exact shared ivars in ARM64 registration and callback
rather than assigning names from branch order. The #109 receiver branch
calls the recorded getter/comparison/release paths without a physical
setter. Internal 110 construction is verified separately. Signed installed
A/B confirms real public addresses and names, 110 absent, #108 sustained
response versus #109 negligible held change on the archived cue. Its
legacy filenames "mix"/"amount" are NOT source identities. No alias or
control inferred from filename. The private playable ARM64 SHA 6d163d79…
and genuine signed installed AU SHA 1d6edf2b… are separate pinned digests,
not claimed byte-equal. No original binary or audio in this archive.

Replay using private checked artifacts: check.py BUILD9_ARM64 SIGNED_TGZ.
