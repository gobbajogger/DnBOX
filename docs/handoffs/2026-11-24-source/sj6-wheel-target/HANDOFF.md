# SuperJ6 physical pitch-wheel target

Exact source: controls-init-only/source-ui-pitch-wheel (f8576d34).
One file, main_fm1.c. Applies after the earlier Synth One correction.
The SuperJ6 part's real performance pitch bend is ID16; fallback instead
sent the Chicago target. The corrected helper selects the shared, already
used MIDI bend identity from the bounded committed engine bank. In-flight
physical gesture keeps its original part through a screen part switch and
the existing race guard stays intact. Chicago, Miami and Synth One keep
their proven IDs; invalid/unavailable banks keep the legacy fallback.
No changes to controller range, calibration, damper, storage, MIDI
transport, scopes, or hardware. The comment no longer suggests all
engines share a common +/-2 range.

The fixture extracts the production wheel handlers verbatim. It supplies
ADC endpoints and runs the real host engine queue, held-note SuperJ6
receiver and keyboard ownership on both owners. The previous source
missed both SuperJ6 endpoints for both owners and reported the wrong target
ID on both. The tested patch reaches sustain motion's underlying existing
pitch receiver and keeps the peer Synth One target unchanged. Host standard
MIDI packets compare the same endpoint/center macro, plus zero side
effects in persistent Sound and the host write counter. This is not a
physical device, native, audio parity or installation claim.

Replay with bash run.sh green, or bash run.sh red for expected failure.
