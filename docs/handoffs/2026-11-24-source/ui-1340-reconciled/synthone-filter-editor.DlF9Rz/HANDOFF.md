# Synth One filter type and ADSR pitch tracking

The source-only patch adds a two-control FILTER MODE page to the exact joined nine-control checkpoint. It uses the existing compact Synth One editor, physical detents and page renderer. The existing FILTER, FILTER ADSR and AMP ADSR bindings retain their order and behavior, and the LINK toggle continues to keep an independent FILTER envelope.

## Original source authority

| Editor | Original source and meaning |
|---|---|
| FILTER TYPE | `filterType` is stable ID 115. The original kernel descriptor bounds it to 0–2 and defaults to 0. The preserved voice selects its Moog low-pass at 0, Butterworth band-pass at 1, and Butterworth high-pass at 2. All three labels reflect those branches. |
| ADSR PITCH TRACK | `adsrPitchTracking` is stable ID 149, with domain 0–1 and default 0. The preserved voice multiplies amplitude by a pitch-dependent factor before filtering. The editor shows 0–100%. Tracking at zero leaves the factor at one, and more tracking attenuates higher notes. It does not move the cutoff or repurpose the FILTER ENV control. |

The relevant original sources are [parameter IDs](../../s1-audit-data/original-inputs/closure/master/AudioKitSynthOne/DSP/S1Parameter.h), [parameter domains](../../s1-audit-data/original-inputs/closure/master/AudioKitSynthOne/DSP/Kernel/S1DSPKernel.hpp), and [preserved voice law](../../s1-audit-data/original-inputs/closure/experiment/probe/portable_voice.cpp). Their exact hashes and the independent reader-side voice witness are in `SOURCE-AUTHORITY.sha256`. The candidate applies targets to the existing compact render law. A held change invalidates the coefficient countdown in the existing editor path so the first sample in the new mode uses its coefficients.

## Storage and compatibility

- A/B still use the Synth One rows at `T_303[6–7]`, marked only by the typed Synth One bit 22. The existing nine positions and first 11 bytes keep their representation. Mask bits 9 and 10 claim the two appended positions. On the first filter edit the pair must be zero, so previously unowned data cannot be overwritten.
- A marked oscillator-only record still validates, copies into a note lock, and resets using its original 11 bytes. A filter-authored record uses 13 bytes. The note-lock tail starts at byte 49 and still fits its 64-byte lane. Existing oversize, unknown-bit and unowned tail validation remains fail-closed. The project layout, revisions, shared ARP and SuperJ6 rows retain their formats.
- The previous exact checkpoint loads complete oscillator-only projects including locks and opaque appended bytes. It treats a filter-edited base Sound or a filter-only note lock as unsupported. All six stored project objects stay byte-identical after the refused save. The new build round-trips those filter projects on both owners.

## Host evidence

- Both owners passed 33,234 combined checks. The complete real UI exposes each mode and both tracking extremes. Held edits change at least half of the 4,096-sample owner window with zero peer differences. At maximum tracking, the measured PCM energy ratios were 0.853182 and 0.517702 at MIDI notes 48 and 84, relative to the respective 0% captures. Tests cover FILTER ADSR independence after unlink, typed capture, playback and release, reload, bounds, opaque legacy bytes and preset replacement.
- The previous reader passed 258 checks on complete saved project bundles. The first-sample compact coefficient fixture failed before the refresh and passed after it.
- INIT and all 694 authenticated factory programs produce byte-identical untouched dry/wet PCM over 1,024 frames each against the exact nine-control baseline. In all 695 programs both targets restored their exact factory values after temporary overrides. Existing factory type values were 599 low-pass, 60 band-pass and 36 high-pass, with 17 nonzero tracking values. No private preset records or program/PCM files are in this packet.
- The existing oscillator editor, factory editor, Synth One FILTER LINK, mixed Synth One/SuperJ6 storage, SuperJ6 sync and PWM2, and ARP/ARP keyboard tests passed. The paired four-engine legacy run matched all 49,152 stereo frames.
- Four direct 240×240 captures under `panel/` show the mode values and 100% tracking.

## Exact replay

Apply `synthone-filter-mode-track.patch` only after the preceding [nine-control join](../synthone-sync-join.s979TF/HANDOFF.md). Its patch SHA-256 is `554049fcda50b615f35f58df535db06044f06608c02f96f9205c4dd1ad5fbf5c`. `PREIMAGE.sha256` pins the corresponding source, and `POSTIMAGE.sha256` pins the filter candidate. The replay also accepts the exact donor one step earlier and applies the prior patch privately.

```bash
TMPDIR=/dev/shm bash replay-host.sh /path/to/exact/checkpoint /path/to/authenticated/s1.pack
```

The build compiles the product compact DSP configuration on the host, including its eight-sample coefficient rate, without use of the native runner. Target flash fit, XIP neutrality, native timing, and the Saga ARP combination are unverified for this patch.
