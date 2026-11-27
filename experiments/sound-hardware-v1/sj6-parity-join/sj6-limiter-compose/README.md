# SuperJ6 Rage: dry branch routing, not compressor parity

Authorized old supplied Build9 AU SHA a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1.
The old executed wrapper receipt in recovered-sound's source-profile/old-rage
traces TempoDelay → dry RageComp → sum, and local mix 1/0 in stopped state.
A separate source wiring receipt establishes TempoDelay → gated HPF →
Costello → wet Apple PeakLimiter → return → that same sum. Selectors in
old supplied binary independently rebound from Mach-O fixups by selectors.py:
dry threshold, headroom, master gain, and Bonus Release writes AttackDuration.
The separately bundled AKDynaRageCompressor symbol is not evidence that
this dry AKCompressor receiver uses that C++ kernel. Native source-profile
Raw amount .0625 repeat previously observed strong terminal attenuation;
actual authenticated native leaf input/output is not present locally,
and the private Apple active DSP has not been recreated. No wet limiter
implementation, curve fit, makeup, or guessed DSP was added.

routing.patch only changes adapter/sum placement behind
DNBOX_SJ6_DRY_RAGE_PARALLEL. It leaves the existing compact dynamics formula
intact and covers the currently implemented additive Type II/no widener or
pan combination. The nonadditive Type I model and independently unsettled
Fatten/pan ordering remain explicitly out of scope. The revised adapter
does not feed its modified dry samples back into Room construction.
build-check.sh extracts both real compact leaves from exact installed
candidate source and runs 50,319 signed stereo frames against independently
composed old dry Rage + old Room return, including closed send with retained
state, reopen, signed zeros and fully stopped dry Rage. Room state and
limiter state bit-identical to independent branches. This is a graph
verification, not a comparison to Apple processor PCM or end-to-end parity.
