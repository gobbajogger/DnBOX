# Restored live Decay and Noise Tail

The actual managed 7f ui-drum-glyph failure was not only a stale crop. The corrected current R22 crop still failed after a real engine hit: the renderer ignored the live envelope and age. The repair preserves the approved idle curves, numeric units and input routing, restoring the prior -60 dB remaining-tail law, dim idle ghost, Noise Tail integer modulation/age seed/fifth-column fills, and 300 ms return fade in the R22 viewbox. One existing 96-byte per-voice timestamp array is shared, not duplicated. Native resource fit still requires exact integration measurement.

Actual final full production host fixture: 113,424 checks PASS, including real 909/808 Decay and Noise Tail, clock-only noisy age change at unchanged actual envelope, and direct byte-identical settled return. The noisy fixture sets actual Tone and Decay to 127 to retain a live voice at 150 ms; the initial factory-tail assumption ended too soon and is not a product failure. Cold factory idle sweep directly compares 63 raw files, 7,434 frames, 45,674,496 bytes with zero differences, not hashes alone. Before and after original source manifests remain unchanged after execution. Twelve actual full-frame captures per side record engine live/age/envelope; raw files and images remain local, not included here.

Reproduce host regression: `SMC_UI_TEST_FILE=tests/host/ui_drum_glyph_test.c SMC_UI_OUT=build/host/live-tail sh tests/host/smc_ui_test.sh`.
Reproduce capture in separate before/after sources: create an absolute output directory, then set `REMOTE_GLYPH_OUT` to it and `SMC_UI_TEST_FILE=tests/host/fwui_drum_idle_live_capture.c SMC_UI_OUT=build/host/live-tail-capture` for the same wrapper. It writes settled canonical glyph buffers and full-frame PNGs locally. Compare corresponding raw buffers directly. Source manifests describe executed donor lineage, not a future combined integration SHA.

Apply runtime-only.patch over integration's shared hooks. Do not overwrite its full ui.c. This is bounded regression evidence, not a complete screen inventory, all raw/dependency/live combinations, full geometry/animation acceptance, device result, native budget or Local gate PASS. Prior failed source/test logs remain historical evidence.

The verification JSON preserves original local log hashes; published logs normalize the workspace prefix. publication-receipt.json separately binds the normalized delivered files.

Capture logs are line-preserving .part1–.part3 chunks; concatenate in numeric order. This splits technical text only, not denied artifacts.
