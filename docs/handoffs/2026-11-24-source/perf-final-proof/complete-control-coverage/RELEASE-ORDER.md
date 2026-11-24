# Engine-first delivery checkpoints

Romario's latest order: make both compact engines actually behave like their
originals and fit, then tune their sound. This changes sequence, not scope.

1. Map the original user-facing controls, owner, range/unit, switch semantics,
   signal path and source for each engine. All 64 claimed SuperJ6 controls
   must have a traceable definition; cover the full Synth One product set,
   distinguishing app-wide parameters, gestures and true engine controls.
   Do not count preset-only storage as a working control. Keep AMP/FILTER
   LINK semantics on both engines and the real step/GATE and held-pad entry.
2. Implement the missing control families with shared, measured code when
   equivalent; keep the separate state and differing original laws. Host
   qualify physical edits, parameter-specific audio/transport behavior, both
   parts, save/reload, peer isolation and exact bypass. Verify controller
   affordance, not just a register path. Fold proven table-driven UI layout
   into exact integration; follow UI guidelines across actual pages.
3. Build the same exact combined candidate for Elite and Pro within the
   real app map and pool budget, no PSRAM. Do not promote the disposable
   companion alias. If paired data is needed, require the validated map,
   safe migration, whole-region verification and a rollback proof. All
   programs/banks must remain reachable as specified; UI and DSP cannot
   silently discard original nondefault sound values.
4. Native qualification for one exact candidate: both audible BLE boots
   and UI, stock-reference parity where unchanged, zero late/dropout and
   <=10,909 us in calibrated heavy scenes at all required offsets, with
   both engines and original arp active; no stale baseline substituted.
5. Refine A/B against the original DSP after structural fit, not isolated
   preset compensation. Dry first, then effects one at a time, then whole
   wet patches. Keep six-pair Synth One null/spectral acceptance (Pad and
   Organ release tails included); honor approved SuperJ6 original-AU A/B
   timebox independently of engine, control, preset and bypass requirements.
   Recheck final size, timing, sound and complete integration after changes.

As of this checkpoint: PWM2 + data-driven pages save 1,024 B on both models
relative to the same featureful PWM2 code. The normal map remains over by
12,184 B Elite and 12,176 B Pro. The disposable paired layout has +1,116 B
and +1,124 B, pool safety margin +512 B on both. This is a linked scoped
checkpoint, not installable and not the full product. The last alternate
audio candidate was rejected at 14,741 us and 129 xruns on Elite Miami+arp.
