# SuperJ6 dry Crusher leaf at zero-decimation physical tuple

The saved **native Apple** `aufx/dist/appl` capture (version 67072; 44.1 kHz)
contains the needed alternative to the older 0.44387 fixture. Twelve zero-wet
arms have readbacks Apple **#3=0, #4=0, #5=100, #14=−0.899999976, #15=0**.
This uses the captured built-in unit, not an interpretation of SuperJ6's
public ON/OFF param as graph connectivity. No new native render was needed.
Headless component discovery returned no component for a fresh AU test, so
the replay uses the archived signed leaf outputs.

Regenerate only the original authored sine stimulus using the same Swift
Float32 operations. Validate each output SHA, signed channels, equal
before/after physical values and callback total. Then compare the complete
waveform to `y = x − xPrev + .995*yPrev` without delay or gain fitting.
Across 835,584 stereo frames at the twelve recorded amplitudes, worst
signed residual is **−128.44 dBr**, largest absolute error is 5.961e−7.
Simply copying input (mathematical bypass) leaves **−21.94 dBr**. This
independently supports the same DC state at the new tested tuple.

Replay: `sh replay.sh SJ6-Crusher-Source-Mapped.tgz`.
Archive SHA256 `c117eb6f2c51131fead72a31313f7d5d79aa2e54d05fb196b3016cba016de55e`.

This is a physical leaf result for one tuple and one class of input shape,
not a claim that the installed whole SuperJ6 app selects precisely that
graph tuple, that its inactive control never writes, or that the full
Solidify release matches. No synth clip, original sample/binary or target
firmware is included. No fit, timing or publication claim.
