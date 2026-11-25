# Editable original-clock ARP PARAM host experiment

Exact base source-s1-reverb-lfo manifest
f74afed6ce7fc9087c4b1e3a8b2e53a3e5e5e2ad57fc1ea878ce5c49399b077b.
Patch requires no other patch and cannot be enabled outside X0X_HOST.
Run `sh perf-final-proof/s1-arp-clock-panel/replay.sh WORKSPACE_ROOT`.

Physical SELECT moves from Synth One ARPEGGIATOR to adjacent ARP PARAM;
K1 ARP RATE edits 1–200 BPM (integer hardware choice, not a claim of every
fractional slider value), K2 CLOCK FACTOR uses the original 19-stop order.
Existing separate ARP SEQ grid still has STEPS / INTERVAL / GATE and existing
stock ARP SPEED remains for transport/fallback. Both requests are serialized
per owner and applied at the audio boundary, with bank/owner guards. Merely
opening the page does not enable the new timing or edit project data.

Actual physical knob input and live key callbacks yield exact note onsets
137/206 for different factors on the two owners and render non-silent audio.
The test starts actual transport from an independent sounding note, holds
through STOP, releases, and reproduces original clock timing on the next
physical press. It also switches from independent clock to stock 55% gate
during hold and proves the key latch and ARP sounding view clear on release.
A separately initialized transported hold/STOP comparison is exact PCM against
clock not configured; legacy ARP PCM proof is in sibling s1-arp-clock-app.

No typed wire or recall. Candidate A4 owner slot bytes 7–9 were reserved by
root/sibling coordination but unused. Need actual shared typed parser/base,
cold/project reload and old-reader no-write proof before storage; an integer
SMK profile can probably represent integer BPM losslessly within two bytes,
but this is not lossless storage for the original continuous binary32 range.
Neither engine library nor production pages or builds were changed.
