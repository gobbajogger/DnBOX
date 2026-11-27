# SuperJ6 Normal path: first divergent stages

Diagnostic only, not a full engine or product match. No fitted gain, timing shifts, preset edits, physical device writes or compiled artifact shipped.

The copied compact originally filtered the oscillator, then applied its amplifier envelope. Current original role taps show envelope/gain before the Normal IIR. The voice attack depends on that order. Moving only the same already-evaluated AMP scalar (not velocity, coefficient law, mode, fader, or state lifetime) before the compact Normal filter improved the signed attack residual from −16.12 to −30.27 dBr and attack level from +0.655 to +0.054 dB. This scope is Brainwave warm physically isolated Mono OSC1. The guard only applies to typed SuperJ6 Normal voice path.

The Normal recurrence is separately wrong. Feeding the same original input into the stock compact Normal filter caused output to diverge on the first nonzero sample; stock signed residual was about −33 dBr at 211 Hz, −31.4 at 300, −28.5 at 600. A DF-I bench implementation reproducing original float state and ordered higher-precision accumulations reproduced every signed source output word for fixed and two dynamic coefficient tapes with identical source input. The held-out edit frequencies were 390 and 750 Hz, not the tuning cue.

| Controlled compact path | Attack | Late held, dynamic | First release, dynamic |
| --- | ---: | ---: | ---: |
| Stock | −16.12 dBr | −26.88 dBr | −31.45 dBr |
| Original AMP order + exact DF-I | −34.49 dBr | −38.00 dBr | −38.26 dBr |

Held-out edits: late −26.04→−37.83 and early release −31.35→−38.25 dBr. Exact filter with old AMP ordering still had about −16.31 dBr attack. No claim that either explains the full installed AU level/release mismatch, and no all-source/voice match. Factorial receipt digest prefix `7db3903ca`.

The staged AMP-order patch `2a7455667259b6d9bb020b025451e59cfc72a39e` linked on isolated guarded native Elite and Pro, costing 48 B APP each. 1420/1432 B APP free, pool 932 B usable. ELF `875a3e6b1ceda6e235afeb138cda434d03de5124066a938d9178a61406818a82` / `384c7f5b5258b14e64e3ca0f46467fdc0ba949852581d7fea457df1a54ed0f10`. Not executed with populated SJ6 on native; no promotion. Need precise live Normal coefficient mapping, two extra input history words per owner, mode transitions, multi-source clips, and timed native recurrence.
