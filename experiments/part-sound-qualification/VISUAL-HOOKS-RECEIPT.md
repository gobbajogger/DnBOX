# Test-hook qualification on the full Part Sound canvas

No links, shipping DSP, frame renderer, or acceptance threshold changed by this commit.

- Correct the traced anchors for the actual cached Part Sound painter and account for the row base of each compact canvas. Preserve real geometry, drawn glyph and duplicate-value detection.
- Trimmed host run (framebuffer/audits, no complete PNG archive): 1,545 actual named states, 36,205 text audits, 19,771 presence checks, 249 rendered glyph checks, no findings. Compact first slice: 46 screens, zero layout overlap or check failures.
- Existing overpaint + duplicate-value negatives on the same binary failed as intended, flagging 249 damaged glyphs and 2,212 duplicate values. No thresholds relaxed.
- Native isolated Elite emulator positively reached actual 808 Part Sound via one physical algorithm detent after opening Part Sound, with both stack guards intact, zero dropouts and unchanged virtual NOR. Separate 909 and Synth One panel proofs and timing suite are not replaced.
- Correct the legacy Stop test to respect physical key-up as the release event. Targeted normal integration: 337,756 checks passed. The default official sanitizer gate remains unqualified in this runner.
