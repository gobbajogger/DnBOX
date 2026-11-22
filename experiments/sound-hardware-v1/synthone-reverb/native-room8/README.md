# Native two-owner Synth One reverb

The attached patch applies to `bf48ce83bea6f8d332dddff11e5407c4fd744fc6`. It builds the full eight-line reverb, high-pass filtering and both stereo dynamics stages on Elite and Pro. Each owner has 25,296 bytes of dedicated external history distributed across four disjoint regions, in addition to private history inside the selected synth. The firmware retains the production 808 echo range. [Source patch](full-rate-room8.patch) · [Source tree and build inputs](manifest.json)

| Eight-second populated project | Elite peak µs | Elite late / xruns | Pro peak µs | Pro late / xruns |
| --- | ---: | ---: | ---: | ---: |
| Chicago and Miami | 11,473 | 0 / 0 | 11,634 | 1 / 1 |
| Juno and Synth One, original room disabled | 10,067 | 0 / 0 | 10,763 | 0 / 0 |
| Juno and Synth One, original room enabled | 14,041 | 64 / 64 | 14,298 | 97 / 97 |
| Two Synth Ones, both original rooms enabled | 15,910 | 133 / 133 | 15,915 | 132 / 132 |

Both packages linked with 8,376 raw pool bytes free. Remaining application flash was 2,932 bytes on Elite and 2,940 on Pro. A separate build reproduced each ELF and FWSC. Every replay passed the post-boot PCM-length and guest-fault checks. The wet cases verified saved settings, advancing delay-line pointers and nonzero private history on the guest. The shared cache configuration repurposes seven data ways and leaves all instruction ways active. Cache-way performance on physical instruments still needs verification. [Native package identities, complete counters and RAM evidence](receipt.json)

This measured image predates the energetic-tail and selector fixes. The corrected ordinary source passes 90 checks covering independent owners, hardware controls, switching, project restore, recorded edits and undo/redo. In the original host graph, the eight-bit native stage passed 694/694 final outputs and 692/694 isolated returns. The nine-bit version passed all 694 for both measures, including the editable seven-control panel and 34 fully wet programs. The whole factory voices were provided by the original host graph. Ordinary preset selection still supplies the INIT voice. [Corrected source and result](../bound-stage-result.json) · [Implementation and replay](../bound-stage-proof.tar.gz)
