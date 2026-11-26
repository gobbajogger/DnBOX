Deterministic current Raw J106 source postvoice owner, no new audio render.
Run graph.py with prior authorized current ARM64 source bench/PYTHONPATH.
Verifies exactly saved dry control identity; current #8, #130 zero raw OTA.
Actual current app callback .0ef80c maps to exact 1 via actual Swift
AKMixer setter .47b638, measured at the native mixer property boundary.
Positive .5 gives .858578622. No independent Apple PCM modeled.

Current CoreGeneratorBank constructor locates lane objects before
OscSection array .2b8914 (MainA x29-.1c8 from .2b8178, MainB x23 from
.2b8218, Sub x29-.1c0 from .2b8024, Noise x27 from .2b82b0). Executed
real array-packing instructions produce OSC1, OSC2, Sub, Noise in that
order; these objects flow into AKMixer initialization at .2b895c, and
it owns oscSection (+metadata .7e3be0). Both allocated oscBooster objects
do receive exact .5 Swift setGain in actual constructor .2b8f70/.2b8f84
but aren't that array's operands. No guessed postvoice attenuation.

Previous authenticated terminal vs actual compact stage evidence carries
unfitted, unchanged .34313184 master. Largest of retained late stage-only
signed errors is Raw strip +17.126 dBr post HP in 50–100ms after first
Note Off (RMS .001974); pre HP was zero there. Audible attention: 16–50ms
actual candidate final signed +9.571 dBr (RMS .002843), and first 16ms
+3.933 dBr (RMS .023409). These diagnostics compare against genuine
terminal dry, not a same-input internal original node. Without private
installed pre-J60 or common-input pre/post HP audio the mismatch cannot
be uniquely assigned. No patch/new gain justified. No uploaded original
app, samples, screenshots, or raw terminal PCM.
