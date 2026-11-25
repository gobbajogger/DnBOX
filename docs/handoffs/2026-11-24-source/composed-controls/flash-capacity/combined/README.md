# Combined virtual slot layout and exact preset-edit witness

This is a host-only implementation. It creates a JSON extent plan and verifies
actual factory audio bytes in memory. It does not create flashable output or
modify existing ownership, projects, sounds, or installers.

Run `layout.py --stream-helper ../stream_storage.py --909-header <generated
x0x_drum_samples.h> --break-assets <firmware assets/default-breaks>`. The two
published board-generated headers have identical SHA-256
`6fd7fe81e4af56afe59bc0fde462c5c237d1e86c332a3bb163a7f21eee619c50`.
Re-encoding verifies all 55,275 909 ADPCM bytes and 109,274 BREAK ADPCM bytes
exactly, through bounded Huffman blocks including the restart indexes. Actual
BREAK archive hashes are checked. This reproduces 57,120 B FDATA including
reserved non-sample payload and 46,037 B Slot 2 default BREAK with headers.
Both board inputs produced identical aggregate results.

The planned existing Slot 2 prefix is split into BREAK 0xD6000..0xE1FFF,
8 KiB payload 0xE2000..0xE3FFF, and 8 KiB fixed duplicate directories
0xE4000..0xE5FFF. The new FDATA boundary is 0xE6000. The conservative joint
journal can add another 8 KiB at 0xA5000..0xA6FFF, and the expanded option
another 8 KiB at 0x9D000..0x9EFFF. These are separate migration prerequisites,
untouched in production, and are conditional alternatives. FDATA is not
then counted a second time as preset capacity.
 
| With unchanged locations for other assets | Potential preset payload before update reserve |
|---|---:|
| Compressed 909 and BREAK only | 8 KiB |
| Plus conservative 8 KiB journal | 16 KiB |
| Plus expanded 16 KiB journal instead | 24 KiB |

Whole-bank transactional replacement in the tested largest-bank edit used
five extra 4 KiB sectors. This layout cannot accommodate the 176 KiB bank
corpus. The 140 KiB best-case bound discussed separately assumes significant
unproven asset relocations; it is not what this layout demonstrates.
 
`bank_delta.py <preset-offline-dir> <private checkpoint source helpers>
<J6presets.zip> "<Synth one.zip>"` replays the exact 1,546-preset corpus and
the existing inserted novel parameter word plus delete case. Instead of
rewriting a 19,767-byte encoded bank, it stages a **59-byte delta** and a
24-byte directory record. It verifies all old/new prefix combinations
(1,500), binds the delta to one exact base bank, and checks all 24 other
banks do not change. This proves small-edit staging can be one sector rather
than five; it does **not** reduce the 176 KiB resident library, guarantee
arbitrary whole-bank replacement in one sector, implement cumulative delta
compaction, or replace verified original-data backup.
