# Fit-first native link increment

The existing joined AutoPan/VOICE/Gate/RANDOM2/Delay/Filter Velocity
diagnostic links at 324/336 B of free APP and 504 B of free POOL above
the 8192 B reserve. This change is a separate guarded linker/profile
step, not additional release or hardware authorization. Timing,
firmware behavior and device migration are not qualified at this step.

Reorder five named uint8 members within the non-serialized
S1SequencerParams. The only change is the runtime ABI layout; all
callers and both owners compile together. No project schema edit,
no scheduler law change, no removed setting. Actual POOL recovery
is 16 bytes, giving 520 B above fixed reserve on both models.

Move only the existing exact nonlinear tanh table, the two AutoPan
read-only arrays, and the existing velocity detent table into the
already-guarded FDATA diagnostic span, not into any BREAK slot
or protected user storage. The link validates each named section's
exact size and range and binds every byte, including the data already
there, to the same app/model/CRC header. Both actual links consume
60,944 bytes of the existing 61,440-byte FDATA span, leaving 496 bytes
of that region, 6,024 APP bytes on Elite and 6,036 APP bytes on Pro.
Older occupied releases still cannot take this image by default. No
emulator, package, installation or hardware step was performed here.

Apply params-layout.patch with fuzz=0 on the exact joined logical
firmware donor. Run link-fit.py using its current native build
wrapper/profile and generated assets, targeting each existing model;
retain stock APP and FDATA boundaries. Artifact stays local until the
complete control set exists, fits, and later verification is actually
passing. Do not confuse this fit step with completed parity.
