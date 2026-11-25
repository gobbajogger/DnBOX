# Existing single FILTER LINK per engine: current Init-only host

Exact source-ui-pitchmod cb20b1ab, UI SHA-256
2ac88ffdbaa9c71080d5bed5d5fb2a27f55cb516147a9b0d7a10a670e17cc72e.
Both current synths expose one and only one LINK in PART → FILTER → K4,
not an AMP LINK page or mode. Tested actual physical SELECT/detents and
the existing queued state/persistence. The owner intent is unchanged.
No product source/registry/skill/device changes.

- Synth One, both A/B, 112 checks: physical LINK changed PCM in 8,014/8,192
  sample words for each owner. ELF test wrapper observes the real S1C
  renderer running a keyboard note: four underlying filter ADSR command
  words equal AMP words while linked, differ when unlinked, and the
  independent knob settings are retained. Single-LINK cold toggle,
  ON/OFF project restart and full engine PCM exact per corresponding arm;
  peer Sound rows and independent runtime physical ADSR/flag unchanged.
- SuperJ6, both A/B, 458 checks: sole page/slot/label and exact SMC
  target; both live layers select AMP sustain ON vs independent FILTER
  sustain OFF, confirmed in actual keyboard-note envelope state, peer
  raw rows/parameters and own independent AMP/FILTER words unchanged.
  Verified ON/OFF saved/read through real project writer/reader. The
  isolated dry Juno scalar signal from the mounted runtime's exact state
  is bit-identical per corresponding reload and changes in 7,938/8,192
  words ON vs OFF. Full mixer reload bits for this SJ6 fixture are not
  qualified (prior stream state affects the first samples).

Functional follow/restore is not a complete clock equivalence claim:
  Synth One uses separate EGs and may modulate AMP decay in its renderer.
  SuperJ6 core latches timed stage targets at entry; already-sustained
  stages can update immediately. Authenticated original ID116 ordered
  callbacks, held ramps, short Saga gate/release, dependent clocks,
  velocity and sonic parity on native are separate work.
  No reason was found to alter the existing mandatory LINK behavior.
 
Replay on an isolated exact base: copy both fixture .c files to tests/host,
run run.sh s1-one-filter-link.c <scratch1> -Wl,--wrap=s1c_render and
run.sh sj6-one-filter-link.c <scratch2> -Wl,--wrap=juno6_init from that base.
