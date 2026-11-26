# +240 APP bytes linked on unchanged scheduler, UI page batching only
Cold exact patch 7f7529e652622f943c3093ed7abf16f20b5687fd0aeca616ea28c4b0efe9b313 from e8da020d/source witness, with 32 exact extracted ordered descriptors. Strict original ui.c source pin 8cbfde48…, isolated changes only to two owner configuration tables/call batching; no DSP/terminal changes. Thin OFF was recompiled byte-for-byte, same serial scheduler and warm J60 flags as the released-to-diagnostics original test anchor. Not a release.

| Model | OFF APP | ON APP | Physical ON ELF SHA256 |
| -- | --: | --: | -- |
| Elite | 520 B | 760 B | a58754a83c2edacea2db57cf4d6b20da552e1dc42f426c8dac168ba417b017f3 |
| Pro | 532 B | 772 B | 9be070f0a30662b205b276427680aab51594f614abed5645a2e8faa811c0fa9c |

Raw POOL 8,756 on all, +52 beyond floor 8,704; ordinary BSS unchanged (gap 156), XPOOL unchanged. No firmware/USB/board use or playback/emulation. In-memory package projections verify FDATA, FITTAB and MORPH bytes unchanged and exact SLOT3/XDATA memo pointers by pointed strings or recursively equivalent same named/sized descriptors. Only numeric compiler disambiguator on sj6_knob_desc.wave allowed to vary, with exact size and content tested rather than blanking pointers. 288 SLOT3 and 41 XDATA changed pointer words per model (not changed values). Every one of 73 assembled pre-link engine/DSP/seq/original object SHA pairs is bit-exact: 6 DSP, 62 original, 4 sequence + engine. Final linked function numeric addressing can change; those addresses are not an independent post-link PCM test.

UI serial OFFs retained: Elite aa2cacb3…, Pro 34537d27…, exact logs and map receipts included. Both never-install ON ELFs were backed up (each roundtrip cmp and raw SHA verified). No release artifact, no builder publish and no further link.
