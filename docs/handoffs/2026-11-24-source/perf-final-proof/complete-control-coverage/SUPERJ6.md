# Complete SuperJ6 control coverage: initial ledger

Release requirement: 64/64 actual product controls, all effects audible with
active stimulus and bit-exact bypass, all 640 presets selectable. The Build 9
catalog (0.999.8) has 218 app identities, 170 apply positions, 167 distinct
applied identities, 174 unique serialized keys and one unknown legacy key.
None of those totals is the exact required 64. The numbered denominator must
have explicit versioned provenance; no inferred “64th” and no invisible
preset field counted as user access. The original compact 16/16 result is
not evidence for 64/64.

| Original IDs | Firmware control / behavior | Best actual evidence | Unqualified |
|---|---|---|---|
| 82/83, 86 | OSCILLATOR wave 1/2, range | scoped physical edit, audible, persistence | original wave retrigger; release integration |
| 210/211, 78/79 | PULSE WIDTH enable and amount on both layers | scoped physical edit, audible, persistence; original receiver 78 later | two alternate modulation receivers proven on host; no simultaneous-route claim |
| 36–39 / 119–122 | one AMP ADSR changes both compact layers | scoped physical edit, audible, persistence | original primary decay, sustain, release and secondary link topology |
| 132–135, 116 | FILTER ADSR and AMP→FILTER LINK, stores independent FILTER values | scoped physical edit, audible, persistence, LINK OFF restores FILTER | exact original normalized FILTER tapers |
| 0 | OUTPUT LEVEL, original post-FX 0.404× transfer | scoped held audible through active chorus, persistence, peer unaffected | original factory preset end-to-end and final gate |
| 88/96/100 + 195–197 | LFO 1/2/3 clocks, separate Hz/sync, 18 divisions | host 11,778 checks including 120→90 BPM held PCM, MIDI Clock and exact settings on reload and authored clock words | original global tempo-source selection/phase-reset and native fit/timing; routes other than independently proven paths |
| 97 + receivers 78/79 | LFO2→PWM1 or PWM2 with shared depth, route and base panel | scoped held PCM for both routes and parts, route OFF depth retained, zero depth bit-exact, persistence | exact original selector/base-word mapping; original second simultaneous route |
| 40–44, 208 | original CHORUS I/II, rate/depth/mix/version | switch and bypass evidence; existing compact chorus stimulus | original active behavior, full editable controls and routing |
| 146 and original sequence/arp group | DnBOX 16-step arp grid, GATE, held pad + key entry | DnBOX gesture host checks | original destination, timing/division/modes and real sequence parity |
| Remaining original identities | consult versioned public source catalog | not admitted by this ledger | control subset provenance, UI wiring, semantics, stimuli |

Sources: the source-only host receipts in ../sj6-knob-persistence/,
../sj6-envelope-panel/, ../sj6-master-panel/, ../sj6-hz-panel/,
../sj6-pwm1-lfo2/, ../sj6-pwm2-lfo2/, ../sj6-lfo-sync/ and firmware repository
experiments/sound-hardware-v1/mapping/results/implementer_brief/public_control_catalog.json.
Audit baseline source was c4c5ccf13999204ea9e82dcb914ce0a9b3b5334f;
receipts are explicitly not claimed on that baseline or a single release image.

Each admitted control needs original ID/meaning/units and source pin, physical
access in SMK groups of four or an agreed arp-grid gesture, intended on-state
behavior relative to that control's neutral state, peer-channel isolation,
save/reload plus preset reset, and actual release identity. Trigger-dependent
controls require note/retrigger or timing stimuli, not generic held PCM.
Measure per-effect enabled difference and disabled exact bypass; then run the
combined same-candidate fit, strict zero-dropout and product UI gates.
