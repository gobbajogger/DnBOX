# Stock SuperJ6 WIDE and offline LeakSanitizer

This is a combined software candidate built with the exact Sound sources from `0ac65800`, Sound evidence through `136483b5`, and FWUI MIX amber merged at `1d727e2b`. Both stock packages were linked with the previous native Remote and reduced drum-size profile plus `X0X_SUPERJ6_STOCK_PRESET=1`.

| Stock model | XIP free | Above the 1,024 B floor | Pool margin above 8,192 B reserve |
|---|---:|---:|---:|
| Elite | 1,032 B | 8 B | 732 B |
| Pro | 1,040 B | 16 B | 732 B |

Both standard package validators, the stock flashmap check and the exact native emulated USB transaction passed. The updated package hashes are in [packages.sha256](packages.sha256). The previous eight-second playback measurements use older linked packages. The exact WIDE wet deadline and the final ten-minute checks remain open.

The production preset selector and cold restore apply the opted-in DnBOX-authored WIDE row before the first note. Original Juno programs remain indices 0–3; WIDE appends index 4 and pattern code E5. Builds without the option retain the four existing choices. The existing project layout is unchanged. Projects using WIDE require a build that understands E5. Mixed legacy and WIDE per-note recalls follow the incompatible-lock path. The 391-check host regression covers both owners, first typed notes and mode-2 chorus, save and adoption, live and saved Wave/Chorus/PWM settings, and note-lock application and restoration. The existing general Wave glyph and Width readout still describe legacy behavior and need WIDE-specific visual qualification before release. The private original 640-program export is outside this source-owned preset.

## Unchanged compact-program test

The offline TCG Debian 6.12.117 guest ran the test as uid 1000 with TracerPid 0. Debian package metadata and the kernel came from signed distro packages unpacked into a temporary directory. Guest network was disabled, and all host compiler, library, and source shares were read-only. The independent clean process exited 0 and the deliberately leaking process exited 23 after LSan reported exactly 123 bytes. The unchanged wrapper also detected its own genuine leak. All 3,591 exact binary32/name checks passed in both ordinary and sanitized builds, and the metadata corruption control was rejected. The wrapper, guest, and QEMU each exited 0. All ten source input blobs match this candidate. [Guest transcript](leak-sanitizer-guest.log) · [Input blobs](leak-sanitizer-inputs.log)

The previous five-sample offline BREAK package evidence still applies to the pinned ZIP. Large packages and raw logs remain local while private Project Sources capacity is unresolved. This packet does not authorize a release or device action.
