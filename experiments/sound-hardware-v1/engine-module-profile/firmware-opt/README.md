# Synth One-only firmware optimization follow-through

Acceptance is a single Elite/Pro candidate with real flash/RAM fit,
source-referenced sound, and zero underruns in loaded held-note playback.
No new result qualifies all three. SuperJ6 stays disabled and live arp
remains enabled. All native tests here are no-device sealed emulator
diagnostics, not an installable firmware package.

On the verified voiced workload, strict-float -O3 for only s1c/s1u with
the exact pitch cache reaches 15,955 / 16,033 µs and 188 / 197
underruns, against 10,809 µs and zero. XIP free 69,908 / 69,916 B and
8,832 B of audio pool beyond 8,704 B reserve. Six host programs matched
the old compact DSP in wet and delay-only renders; the separate -O2/cache
combination passed all 694 programs in the available legacy pack.
Neither verifies 906 original presets or original wet-signal parity.

Moving the source-equivalent recurrent voice loop to executable RAM
with proper call veneers was mixed: matched-ABI XIP 19,042 / 20,143 µs;
SRAM 19,692 / 19,428 µs. The linked SRAM bytes match the boot copy
exactly, but the addresses don't yield a consistent win and consume
1,244 B of otherwise available pool. Reject that relocation for this
candidate.

Under the tight delay cap, the causal four-cell correction adds no
history and mostly retains exact control semantics, but its modest
Hall improvement trades away Deeper Space Pad spectral accuracy.
A long live retime remains a clear miss. Reject the candidate.
A different compact Room topology fits the modest arena extension but
removing six physical feedback lines collapses the measured tail even
at Float32. Do not ship the two-line substitute.

Exact phase shortcuts and shared per-sample morph decode passed signed
host checks (including minimum four active voices and live morph
edits on the latter), but did not improve both native results. Their
held-note timings and sources are in dsp/. The O2 + pitch cache +
hot-inline build reached 16,123/15,828 us with 195/184 underruns,
not a gate pass either. The O3 + pitch cache has the best of these
worst-case two-model peaks, 16,033 us on this single scene.

Host display probe genuinely measures intermediate anti-aliased RGB565
writes in the existing renderer. Across 276 states (408 canvas draws)
there were at most 185 distinct colors per 240×138 canvas; every one
of those 408 draws reconstructed all current pixels exactly using
33,120 index bytes + 512 RGB565 palette bytes. Original frame hashes
are unchanged. This is a host mirror with the raw canvas still intact,
not firmware RAM reclaimed. Canvas-only potential is 32,608 B before
palette lookup, any DMA workspace, and an exact overflow fallback.
Snapshot-safe overflow behavior and actual target CPU cost are open.
The measurement fixture logs existing highlight-layout expectations
without treating them as a display product gate; this is not general
UI qualification. To run against this donor, adapt only the out-of-date
fixture title accessor to page_title(&pg,page), and provide host
backing storage for the native-only lab_bass_half[2] counter.
