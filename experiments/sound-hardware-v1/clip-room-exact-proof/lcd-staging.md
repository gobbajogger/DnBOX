# Exactly reclaim 480 native RAM bytes by giving LCD staging one owner

Base: lane/sound-clip-delay-wrap e00f689dcab38f396dc23325e0f8eef515925489.
Only firmware/src/lcd.c and firmware/src/app/ui.c are modified.
No audio, wet effects, LCD stride, wire transfer geometry, UI cadence, installer,
or memory-reserve change.

Previously native LCD solid fill used private 480-byte .pool lcd_fillbuf,
while dirty-tile packing used 1,536-byte ordinary .bss cv_lcd_txbuf. Both are
foreground producers of the same LCD SPI DMA channel. lcd_fill synchronizes
before staging and makes no foreground callback throughout the fill; its
repeated DMA uses the same immutable content. cv_lcd_emit always synchronizes
before packing, launches dma, and _then_ allows foreground input. Thus even
a nested fill called as part of input must drain the old tile DMA first,
and tile commits after returning drain any last fill DMA before repacking.
The hardware wait does not call back to input. The owner buffer remains
1,536 ordinary B; its first 480 bytes now implement the old fill scratch.
The host LCD renderer (which does not compile lcd.c) retains its private
standalone staging declaration.

A/B using the real original and patched lcd.c with an asynchronous mock:
17,143 fills, 13,333 tiles, 101,010 wire transfers, 13,515,436 total bytes,
identical wire signature 44af578e7af55dc9. Mock snapshots every launched
DMA and checks all pending bytes at its next wait. UBSan clean. Each writer
may enter with previous DMA still running; no extra fence was added by
the test except the existing tile-producer contract.

Pro native:
- .pool 335,360 -> 334,880 B (480 real bytes released)
- pool margin above unchanged 8,192 B reserve 512 -> 992 B
- ordinary .data+.bss unchanged 96,456/98,304 B
- app XIP free 2,608 -> 2,644 B

This is a small safe additive cut, not enough alone for a longer reverb.
Current native e00f689d LCD canvas is still 66,240 B (138 rows), not the
old guarded diagnostic's 23,040 B (48 rows). Rebase the documented old
lossless 48-row route ownership and admit every newer painter incl.
current Clip key route; potential allocation difference 43,200 B, not
a product saving until rebase and tests pass. The previous 10,080 B
figure was 69->48 in that separate diagnostic.

Native .xpool is already wholly committed: second backend workspace 5,120 B
+ independent bass outputs 4,096 B. The first backend workspace can be used
by 808 metal staging only inside the scheduler's current exclusivity proof;
not lendable to persistent effects or unguarded cross-core tasks. Likewise
project_stage is the active Undo baseline outside explicit staging, so its
36,420 B cannot simply be loaned to audio or LCD.

Reproduce the wire test from this artifact directory:
  cc -O2 -fsanitize=undefined -DLCD_FILE='"original-lcd.c"' lcd-staging-proof.c -o reference-lcd
  cc -O2 -fsanitize=undefined -DLCD_FILE='"source/firmware/src/lcd.c"' lcd-staging-proof.c -o candidate-lcd
  ./reference-lcd > reference-wire.txt && ./candidate-lcd > candidate-wire.txt
  cmp reference-wire.txt candidate-wire.txt
