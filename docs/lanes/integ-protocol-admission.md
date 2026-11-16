# Current firmware protocol admission

This is an implementation record, not a rule or permission. RULES.md v2 remains authoritative.

## Exact current source

The firmware composition `0ac93f57a885fc3288be427eacb8c08349bdde0b` has no DNR2 endpoint, remote-owned input plane or current native catalog. The historical adapter is source input only. Its core/wire source provenance does not qualify its integration into this composition.

The current-host census at `fab7f69554f0ec28d8da252bf423c4429700268e` records 326 source descriptor tuples and 978 actual zero/mid/max edits, with 1897 checks. It assigns no protocol IDs. Only authentic current Chicago/Miami synth descriptors are included; legacy Drone/Juno cannot be relabelled as Synth One/SuperJ6. Performance controls are not patch-edit parameters.

## Concrete admission defects to avoid

1. Actual current queue exhaustion: calling the void UI setter with a full engine queue changed the master makeup mirror to 127 while its audio state remained 0 after draining. That is existing UI overload behavior, not a newly enabled endpoint. A protocol adapter must reject before mutation, or genuinely reserve queue acceptance; it cannot report this as QUEUED.
2. Historical scalar SET crosses three revision hooks: `pref_set_impl` -> `mark_dirty`, `engine_set`, and the core's successful SET. One accepted protocol transaction must increment once. External physical/MIDI/project A-to-B-to-A transitions must remain observable independently.
3. Historical duplicate DOWN starts with the core's cached OK and only recomputes status if the held token survives. A removed token can therefore inherit stale OK. The adapter must default a cached DOWN to non-active failure and establish the exact surviving cookie's active audio ownership before returning OK.
4. QUEUED DOWN is acceptance, not audible ownership. The consumer must retain the token, poll the identical request under a bounded send-and-receive deadline, acknowledge only actual OK, and preserve uncertain ownership if cleanup fails. Remote owns this consumer contract; synthetic callback tests do not qualify firmware hooks.

## Checked scalar transaction candidate

The unadmitted implementation validates target/voice/parameter bounds, source descriptor maximum, read-only controls, project replacement, bank identity, pending preset adoption and the original sound cell before any mutation. It holds the existing source queue lock through the one-slot capacity check and existing R_ENG mirror/enqueue path. It returns rejection before mutation, or QUEUED after acceptance, never audio-applied confirmation. It enables no transport and does not establish revision or held-owner completeness.

Required checks include full queue rejection with identical mirror/dirty state, recovery after drain with actual audio application, every admitted scalar's boundary values, out-of-range/read-only/inactive bank rejection, queue cursor wrap, bank transition and project replacement. Independent source review and exact native size checks remain required before combining it with an endpoint.

## Remaining endpoint work

- Generate source-authoritative bank-qualified IDs and a deterministic catalog pin from the actual current descriptors, including HELD read range 0..1 versus DOWN value 1..127.
- Add a single transaction revision boundary and audit every advertised physical, MIDI and project mutation. Missing hooks mean writes stay unavailable.
- Add a separate remote ownership plane, stable cookies and generation/context checks. UP/cancel/lease expiry must release only their original owner, never physical/USB/BLE/sequencer notes; queued release remains unconfirmed until audio retires it.
- Test identical DOWN replay without duplicate allocation, queued-to-active transition, removed token, cancellation races, lease expiry, foreign client cleanup, pending project/preset switch and A-to-B-to-A.
- Qualify actual browser/Lua against the current host adapter and current catalog, then exact combined Elite/Pro resources. Historical resource figures and browser synthetic callbacks are not current admission.
- Preserve existing old-editor routing; malformed claimed DNR traffic must not fall through to firmware maintenance.

Current native resources remain the independently measured 0ac composition: Elite/Pro XIP free 2028/2040 B, pool reserve margin 836 B, RAM headroom 1080 B. New protocol storage/code cannot be presumed to fit. No PSRAM, device writes or unqualified endpoint advertisement is allowed.
