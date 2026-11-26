# First quarantined canvas source patch

`boot-home.patch` applies only to exact donor gfx.c and ui.c (see
`base.sha256`). No native build, flashing, Git, or capacity reduction.
`CV_ROWS` remains 138. The boot conversion is the only default-on change
*in the quarantined tree*. Home is behind X0X_UI_HOME_STRIP_SPIKE; it
runs only the actual default Home routing without active parameter readout,
List/Ask/help, live Home, or SMK Clip ownership. Every other path still
gets the original 138-row painter. Mock and fixtures are not in the
production patch.

- **Boot**: real draw_boot_screen entry sends three 80-row bands, cropping
  the immutable bitmap using original absolute source coordinates. Whole
  RGB565 panel agrees exactly with unchanged original in all 84
  stock/custom/picture, palette and font-fallback cases. Same total
  transported bytes, one extra boot-only window; peak payload
  57,600→38,400 B. No canary overrun or pending-source mutation.
- **Default Home**: the real draw_main route branches after normal Clip
  ownership and readout checks. Captures one bounded, by-value state
  snapshot (904 B stack on host; no target estimate) and paints 80+58
  physical strips with old 69+69 semantic coordinates and the same old
  tile cache. If future visible names exceed snapshot bounds, it falls
  back to the original renderer; must be removed/proven impossible before
  a global ceiling change. Real foreground key press/release during the
  DMA, simulated caller-owned palette/bank mutation, and real ui_init()
  invalidation are exercised. A reset during the last top-strip DMA does
  not let second strip rearm region validity, and the next frame repaints.
- **Callback equivalence**: 24 authored nonempty pattern/chain, palette/route
  and callback cold cases, each 45 actual windows with identical geometry,
  exact pixel payload, complete framebuffer, and dirty hashes. Repeated
  no-change emits zero writes. One bank change emits 26 base/candidate
  windows with the same bytes/order. A pending DMA is consumed only after
  verified buffer immutability; above-82-row canaries stay intact.
  The host uses an asynchronous LCD fake and real cv_lcd_emit,
  cv_lcd_maybe_input(), input(), and ui_init(), not a direct painter call.

Reproduce with `bash ui-canvas-first/replay.sh`. The unrelated old drum
contract test in the exact donor refers to a stale page-name type. Replay
borrows only its prefix (platform/SMC stubs and setup), leaving that
unrelated fixture unchanged and not claiming it passed. No native
stack, flash, UI tearing, callback timing, or recovered RAM is claimed.

Atomic path for the remaining capacity switch: add prepare-once immutable
state plus pure draw to each active main route (MIX meter/history,
arp/S1 typed state, BREAK waveform, overlays/readout, Clip and SMC) while
retaining 138; collect real callback/pixel/bus proofs. Only then switch
CV_ROWS=82, remove/fail-closed all 138 fallbacks, assert all consumer
bounds, and measure both native links, stack/pooling and timing together.
Do not reduce compiled size because this one route can run under 82.
