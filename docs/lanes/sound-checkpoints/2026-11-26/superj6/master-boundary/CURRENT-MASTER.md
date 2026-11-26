# Current official Mac AU: Main Volume
Run check-current-master.py against the installed SuperJ6 AU executable on Mac
(0.999.8 build 9). The script pins the whole file SHA-256 and reads the ARM64
route and Objective-C selectors, without loading or copying the executable.

The instrument's master attenuation field is at +0xc8. The current sender maps
the authored raw value linearly into [0, 0.404], calls AKMixer setVolume:, then
the setter clamps negatives and converts to Float32 for setOutputVolume:.
At Brainwave's authored 0.31501427 the derived setter value is 0.1272657663,
the same as the independently executed older setter. The bench already uses
that law; there is no 4x gain at this control boundary.

This does not prove the current app's absolute pre-master gain, limiter,
summation, or PCM. The attempted direct Mac render again failed with
invalidComponentID (-3000); PlugInKit reported Connection invalid even when
queried in the logged-in user's launch context. The installed file was present,
verified and pinned, and was not altered. Continue attribution from the shared
signal chain and existing authentic captures without fitting a gain.
