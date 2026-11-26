# Remove repeated source-exact Synth One compressor math
Build only the copied candidate Soundpipe leaf, never edit the pinned
reference. Coefficient keys preserve exact Float32 attack, ratio and release
bits, reset invalidates the cache, threshold stays live. Keep the double
logarithm exactly, but compute it once rather than through a repeated macro.
The patch does not cut room, feedback, controls, or dynamics.

Actual target standalone leaf: -40 B code with no state for the log reuse
alone; both arms enabled +40 B code and +32 B per compressor (192 B for six).
The bank test of both arms produced 906/906 exact candidate stereo PCM hashes
against the previously waveform-matched bench. Original reference PCM also
matches 906/906. This tests the full instrument sound on the host under fixed
events; no target dropout/flash/RAM fit claim. Preserve both options and
qualify actual onboard memory plus worst block under the combined load.
