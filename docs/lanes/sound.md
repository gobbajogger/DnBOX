# Lane: sound
Owner agent: Dot sound continuation. Branch: lane/sound-removal-consolidated. Updated: 2026-11-17.

## Status
Approved slot-2 removal extracted unchanged from 7bdc3c786e0e4e0ef4155b486a31ddad59d74ec9 onto consolidated 7b17a64e11731de50f4fe46ef7ebe3948a730a10. This PR is removal/legacy-load mapping only, not M1 completion. Synth One OS11 occupies slot 2; old slot-2 projects adopt init. The slot remains silent until the separately developed renderer passes admission.

## Checks
All affected pre-change product/test blobs equal the approved donor parent; extracted product/test blobs are byte-identical to the donor. User-reported removal, slot and preset checks are reused, not rerun or claimed as new independent evidence. Full exact-head Local gate, independent Review and both-model fit are NOT_RUN. No merge or release claimed.

## Now / next
- Now: independently review this narrow extraction and qualify its combined source.
- Next: integrate approved removal; separately finish playable renderer, factory routing, A/B benches and fit.

## Deletes
- firmware/src/dsp/drone_string.h — approved removed engine.
- tests/host/drone_slot_test.c — replaced by synthone_slot_test.c.
- tests/host/drone_string_test.c — removed-engine test replaced by legacy slot mapping test.
