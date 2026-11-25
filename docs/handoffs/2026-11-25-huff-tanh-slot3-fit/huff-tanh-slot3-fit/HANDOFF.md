# Huff909 + split tanh and guarded Slot3 map composition

Exact donor receiver-final-join. In an isolated clone apply
huff909-fit/huff909.patch then tanh-split-fit/guarded-tanh-split.patch to
the tree. Apply combined-placement.patch to the enclosing baseline
link-guarded-exact.py, not on top of either independent placement patch.
Use run.sh + replay-maps.sh; the run regenerates model-local 909 headers.
The combined wrapper retains existing whole-image SHA/CRC, byte-pins,
stock APP and RAM bounds, and guarded companion range checks. No runtime
or delivery. The map-only audit ran because the hard APP linker still failed;
no linked byte sealing checks past first link executed.

| Same map path | Elite APP over | Pro APP over |
| --- | ---: | ---: |
| receiver-final-join | 11,500 B | 11,488 B |
| Huff source, no refill | 11,548 B | 11,536 B |
| Huff + 4,002 B disjoint refill | 7,544 B | 7,536 B |
| Huff, guarded nonoverlap refill, Slot3 metadata, split-tanh refill | 3,876 B | 3,868 B |

Matched scopes in the combined map, both models:
- XDATA at 0x020a9000, size 0x1e12; unchanged font path, no recovery font moved.
- Slot3 lab at 0x020af000, size 0x7fcc (52 free); verified fresh map disjointness
  for the nine additional metadata regions beyond the same header, A/E and
  stock sample metadata; strict guard and no physical activation.
- FDATA lab at 0x020e1000, size 0xee28 (472 free); existing sample body replaced
  by three exact IMA-nibble sequences compressed into 50,947 bytes incl. three
  one-byte lookahead pads and the shared 64-byte table. Native decoder still
  consumes the exact sequential stream through the stock IMA state step; no
  tests yet. Tanh head and tail retain separate pins, and its four immutable
  UI refill data sections do not intersect the seven Huff refill sections.
- FITTAB lab at 0x020fa000, size 0xf73 (141 free); unchanged.
- POOL 0x51e64 used / 8,604 B free = 412 B over reserve; 16 B cursor cost.
  Ordinary BSS 172 B free. No PSRAM/map-cap changes.

Huff qualifier needed: the default nibble path remains, flag opt-in only;
decode lookahead has a bounded pad but now requires a two-byte fetch and LUT
per sample. Source math, maps, and digest pins are not proof of exact native
PCM, active all-cymbal/max Tune timing, or verified sealed-boot/install.
Split-tanh hot branch has its own timing risk. No runtime/physical device,
release package, Git write or publication.
