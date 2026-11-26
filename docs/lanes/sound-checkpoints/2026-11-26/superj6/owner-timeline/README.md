# Current Mac warm Brainwave: Main/Sub timing versus single gain

Run from workspace root with the independently authenticated archived tapes and
fixed composed suffix: python sj6-newer-owner-timeline/probe.py. Native app
capture is not rerun. No signal offsets, amplitude normalization, or pitch
changes are applied to the candidate. Shape error is a diagnostic invariant to
a positive constant gain, not a candidate filter or fitted patch.

The Mac scheduling trace is one note on at 16384 / off at 38434, then three
coincident key-ons at 49459 and three coincident key-offs at 82534. 49875 is
a prior *older source and typed-owner* commit marker, not observed in the Mac
voice state. Use per-channel fixed 128-frame windows to localize divergences:

| Left window | Main signed correlation | Sub signed correlation |
|---|---:|---:|
| 49715–49843 | −.166 | +.980 |
| 49875–50003 | +.968 | −.176 |

A fixed common level cannot make either localized negative correlation go away.
The offsets make a Main-only global gain, uniform DC error, or single steady
shared gain an inadequate waveform repair. They do not by themselves prove a
voice reset cause, exclude an input-dependent shared stage, or establish the
actual newer Mac voice commit. Sub's earlier prefix uses host HP before actual
Apple Crusher (unlike Main's Apple HP and Crusher); treat it as attribution,
not same complete boundary origin. There is no fit or adoption.

Smallest direct next data: newer same exact warm schedule (particularly
49459–50131), capture each distinct Main voice and Sub *before fan-in/Unison*
with old/current owner phase/cursor, prefilter and AMP prep words once per
render callback. This distinguishes causally if reset/patch source happens
before Apple and shared FX. A genuine unchanged material after 50131 and
unaffected first-release are necessary guards. No app internal audio/state
from the newer build is claimed here.
