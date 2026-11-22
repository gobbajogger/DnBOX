# Independent BREAK SEQ footer review

The aligned source hashes are in [the candidate receipt](aligned-candidate-source.sha256). The tested isolated host checkout used the assigned b995fb73 base and exact copies of the four candidate UI source files. The focused test captured 24 complete 240×240 LCDs covering two themes, three pages, and all four step lanes. In every capture all 16 steps received two-digit assignments 17 through 32 and an independent parameter lock.

The fixture passed 5,602 checks. Assignment changes occur only in rows 220–227, below the step labels and away from the slider and playhead ink at rows 229–237. Lock changes are confined to 96 pixels, exactly sixteen three-by-two notches in rows 238–239. The slot axis uses the same x coordinates as the slider and playhead. Visual inspection at native size shows tight two-digit spacing when every slot is assigned. Adjacent two-digit glyphs can touch horizontally on individual scanlines because their ten-pixel advance equals the column pitch.

- [Fixture](break_metadata_geometry.c)
- [Native test and per-scanline output](break-metadata-candidate-aligned-final.log)
- [Dark native sample](break-metadata-candidate-aligned-final/break-metadata-theme-0-page-0-mode-0.png)
- [Light native sample](break-metadata-candidate-aligned-final/break-metadata-theme-1-page-0-mode-0.png)
