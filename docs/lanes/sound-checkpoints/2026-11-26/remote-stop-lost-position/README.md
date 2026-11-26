# PR #206: confirmed STOP clears playhead even if POSITION is lost

Pinned PR head ac9001e01566fedebb0cc301f3a2090d079c3a06,
exact app.mjs blob 24cf61a06cd2306f548fc7b04e73438bf380582c.
Current protocol intentionally sends POSITION independent of TRANSPORT and
does not sequence same-revision events. Caller must not infer a position
from an audio or transport event, but STOP is authoritative for clearing.

Reproduction uses unchanged exact guarded whole-app C host cable from
remote206-pad-origin, real raw MIDI clock/stop, actual loaded JS DNR3
client, the PR app consumer and a bounded Node DOM adapter. It discards one
real all-127 stop POSITION after frame emission, not a fabricated reply.
TRANSPORT=stopped and button stood correctly, but stock leaves the actual
last running position [5,5,5,5,5]. Candidate clears only cached and painted
positions/beat state on confirmed stopped TRANSPORT. It doesn't synthesize
a position or clear legitimate notes played while stopped. Subsequent real
digital pad creates a live highlight, disconnected while still held clears
it, and later same-cable activity cannot resurrect it.

Reproduce after the positive digital host fixture is built: patch.py
takes exact PR app and output copy; node replay.mjs STOCK_APP fails at the
lost POSITION assertion; node replay.mjs PATCHED_APP passes.
No DOM layout, browser, real USB/ADC or release claims. No deployed state
was changed. A lone missing VOICE off at same revision is not detectable
from protocol; do not infer silence with an arbitrary duration timer.
