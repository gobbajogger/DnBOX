# Decision: test a narrow stock-derived SuperJ6 adapter, with a strict net-cost gate

**A narrower adapter is a credible next experiment, but the existing measurements do not prove it will fit.** Keep the stock DnBOX note-slot owner, oscillator/filter primitives, chorus and runtime interface, and add only the independently required envelope/control behavior. Avoid a second general-purpose parameter API underneath the product adapter. Do not switch production backends on this inference.

The current compact replacement adds **1,928 bytes** to the exact firmware baseline. It misses the required flash reserve by **1,772 bytes on Elite / 1,764 on Pro**. Starting again from the stock image, a replacement may add only **156 / 164 net bytes** before it fails that same reserve. Returning to stock would recover the measured 1,928-byte increase, but would also remove the new behavior. A narrow implementation must preserve the required behavior while meeting that small **net** growth allowance. No new native build or algorithm change was performed for this decision.

## 1. Measured incumbent cost

Exact retained Elite symbols at baseline `76040daa5b52d686a8b7b544296ba5d79610a95d`:

| Stock function | Linked bytes |
|---|---:|
| `juno6_render` | 938 |
| `juno6_set` | 712 |
| `juno6_note_on` | 524 |
| `juno6_init` | 124 |
| `juno6_note_off` | 50 |
| `juno6_all_off` | 34 |
| `juno6_active` | 24 |
| **Named non-chorus subtotal** | **2,406** |
| Chorus init / set / process | 60 / 80 / 374 |
| **Named Juno total, including chorus** | **2,920** |

These are exact function-body sizes, not the entire marginal engine closure. Some observation code is inlined into callers; math helpers, constants, alignment and LTO effects are not all owned by these names. The 514-byte chorus subtotal remains required in this experiment and is not available to reclaim.

Sources: [native symbol table](native-elite-symbols.txt), [stock Juno source](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/dsp/juno6.c), [stock state definitions](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/dsp/juno6.h).

## 2. What the generic replacement actually duplicates

The native linker already inlines and specializes much of the generic core into the facade. “Remove the facade” is therefore not a measured multi-kilobyte saving:

| Comparable retained entry | Stock | Compact replacement | Difference |
|---|---:|---:|---:|
| Control setter, including reached core validation/set code | 712 | 746 | +34 |
| Render entry, including reached core render | 938 | 978 | +40 |
| Note-on, including reached allocator | 524 | 460 | -64 |
| Init | 124 | 244 | +120 |
| All-off | 34 | 78 | +44 |
| Active check | 24 | 48 | +24 |

The replacement has additional separately retained envelope functions: stage entry **464**, settle **112**, tick preparation **124**, sample **84**, on **44**, and off **36** bytes—**864 bytes together**. Those functions implement behavior the simple stock envelope lacks. Deleting them is not an acceptable saving; an in-place implementation would have to provide that behavior with a smaller combined update/event path.

Other identifiable closure includes the extra exp2 body **146 bytes**, `shape` **184**, outlined BLEP **78**, and the compact validator's immutable bounds table **184**. The stock renderer already contains saw/pulse/BLEP work inside its 938 bytes, while the new shape path supports additional waveform behavior. Some code can plausibly be shared or folded back into the existing loop, but functionality and regenerated callers must be counted.

The extra exp2 body's 146 bytes are a hard upper bound on removing that particular body in isolation; there would still be call/link costs, and no sharing experiment has been linked. Log2 is inlined in stage entry, so the baseline's 154-byte standalone log2 size is **not** a measured removal saving. The 864-byte envelope subtotal and 184-byte bounds table are code/data envelopes, not promised recoverable bytes. Do not add these figures into a “savings” total.

Normal LTO already removes unused diagnostic-counter work; explicit counter-off saved **zero additional bytes**. The all-API retention overhead is already absent from the best reachable result. Neither is an available second saving.

Source: [compact linked closure](dnbx_core_7161c21e6d9d773c/compact-variant-native/RESULT.json), [per-symbol sizes](dnbx_core_7161c21e6d9d773c/compact-variant-native/SYMBOL-SIZES.csv), [complete measured resource review](CANDIDATE-RESOURCE-REVIEW.md).

## 3. Recommended narrow boundary

Use one product-specific prepared-parameter owner and one note-slot allocator. Retain the stock event/runtime interface and reuse its permitted oscillator/filter primitives. Add separately owned amp/filter/pitch and layer state only where the recorded contract requires it. A second independently controlled layer must not become a second unrelated allocator: paired note ownership, retirement and steal behavior still need qualification.

Translate and validate a raw control once at its delivery boundary, update the corresponding prepared fields, and prepare pitch only when pitch changes. This has a concrete source-based cost rationale: the generic `dnbx_set_layer` recomputes all six voice increments on every successful layer update, and the facade invokes it for both layers. The stock setter recalculates increments only for its bend update. Avoiding those unnecessary twelve exponentials for a non-pitch edit is a valid design opportunity; it is not a measured native timing result.

Keep the envelope implementation independently written from recorded behavior. Reuse one envelope stepping implementation with independent live state; do not collapse amp/filter/pitch contours or use the previous original-derived envelope adaptation. Existing stock filter and oscillator code are efficient starting points, but their transfer functions and limited waveform/control coverage are not proof of original fidelity.

Raw serialized controls should be separate from prepared DSP parameters and per-voice history. **They already are once per part in the measured generic candidate**: 218 floats plus presence bits occupy **900 bytes**, not 900 bytes per voice. Relocating them does not itself reduce RAM. Allocating two new external raw owners would add **1,800 bytes**, exceeding both the current 1,100-byte ordinary-RAM slack and the pool's 268 bytes above its required reserve/margin gate.

Stock state is 352 bytes plus 536-byte chorus, or **888 bytes** inside the existing 3,720-byte backend union. This shows room for a differently organized part payload; it does not quantify the complete second layer, independent envelopes, modulation and private FX. Preserve that union's total allocation and give cold/raw state an explicit owner within accounted storage. Do not assume serialized project capacity exists merely because the values are moved out of a voice struct.

A bounded next experiment for the owner is a stock-based **envelope seam** with physical prepared fields and corrected observation/event handling. Compare the required envelope recordings and block partitions, then link the exact unchanged full firmware to measure the net delta. Keep the whole-image requirement **≤551,648 bytes**, simultaneous memory ownership, and the complete callback deadline as independent gates. A small component object is insufficient evidence.

## 4. Synth One remains a separate profile

The comparison owner's safe S1 brief requires a distinct signal order and voice topology: oscillator components feed a common note path with **prefilter VCA/velocity and the required four-pole filter**. The current Juno/candidate path applies amplitude and velocity after its two-pole filtering, and its independently filtered layers and SJ6 envelope timing are different.

Share low-level math, bounded storage/event utilities and qualified primitives where appropriate. Do not make one universal SJ6 voice graph stand in for S1 or interpret squared velocity alone as an S1 implementation. S1's envelope/filter, oscillator mix, control identities, private effects and simultaneous cost need their own evidence.

Behavior sources, containing permitted observations rather than implementation code: [SuperJ6 brief](../mapping/results/implementer_brief/BEHAVIOR-BRIEF.md), [Synth One brief](../mapping/synthone_followup/implementer_brief/BEHAVIOR-BRIEF.md).

**Decision status:** worth a narrow isolated experiment; no established flash fit, native timing pass, full SuperJ6 acceptance or S1 acceptance. Existing production code remains frozen. This evidence-synthesis unit is complete.
