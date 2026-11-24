# Control coverage boundary for the Init-only firmware

A control is not complete merely because its name or setter exists. Final parity needs an authenticated original UI/gesture and control semantics, the intended physical control on each owner, audible or event-specific behavior, recall and note-lock correctness, and qualification on the exact release source. The ID list below is a progress inventory of physical bindings with source and host evidence, not a claim that these controls already match every original behavior.

## SuperJ6

Build 9's checked inventory contains 218 engine/app identities, including internal and legacy inputs. That is not a denominator of physical controls. The planned 64 named product-control scope has not yet been established as an exact matching list. Shared AMP knobs are not substitutes for independent original controls; gesture stages and discovery are tracked apart.

Individually mapped pre-Osc1 source-backed physical rows (30): 0, 3, 40, 41, 72, 73, 74, 75, 78, 79, 82, 83, 86, 88, 96, 97, 98, 100, 105, 106, 116, 132, 133, 134, 135, 195, 196, 197, 210, 211.

Newly staged original ID1: oscillator 1 level, separate from its sub, oscillator 2 and composite output. A routed LFO depth or frequency switch is not a substitute for IDs 192–194 original note-triggered lifecycle. ID115 AMP1→AMP2 is not ID116 AMP→FILTER.
Current combined host inventory: 31 individually mapped original IDs with scoped evidence. This leaves 33 target slots not yet accounted for on that inventory, not an exact count of absent panel knobs.

## Synth One

The 150 stable original kernel IDs are not identical to screen knobs. At the Synth One detune source pin, 47 IDs have a direct SMK patch/Crush binding (exact ID set below). The independently declared 55 arp/sequencer fields are not included, nor are the three explicit unused/deprecated IDs and the pitch-bend performance gesture. Another 44 remaining non-arp IDs have no direct binding at this pin; not every one is a separately visible original knob.

Exact pre-Mono/Glide set, sorted: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32, 34, 39, 40, 44, 45, 46, 47, 48, 49, 50, 52, 53, 54, 115, 120, 121, 139, 149.

Newly staged original IDs 20/21: independent Mono and Glide controls, separate from Legato ID120 and AMP→FILTER link. Active path applies a pending topology change before the note onset; strict native timing still required. ID138 is not a working original live effect control in the pinned kernel: setup reads its parameter, but active low-pass delay processing writes zero resonance before invoking the filter; other filter types bypass the stage. Don't add an audible control based on the metadata alone.
Current combined host inventory: 49 direct bindings and 42 remaining non-arp IDs without direct binding, with the same caveats.
