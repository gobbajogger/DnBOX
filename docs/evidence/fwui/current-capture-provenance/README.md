# Current partial capture identity

Both inventories were actually rendered from published af56c8a4ba7b140402db71ba50a4dea6e14fddfe source and unchanged harnesses at that same commit. The selected source subset contains all 1,751 remote firmware/host/tests/tools/assets/experiments Git blobs plus build.sh; each object hash was checked before and after execution. This is a source subset, not a full checkout or ancestry archive.

- Ordinary/global: 172 captured 240x240 states, harness tests/host/ui_screen_inventory.c, Git blob c604b9070f21ed576d113feeedf59c087297dcb0.
- Deep lists: 500 captured states across 57 actual lists, harness tests/host/ui_overlay_inventory.c, Git blob 9cc88021b9d49fec63d0cea134ac7725f845df2c. Actual entry, rewind, selectable-row counts and uniqueness assertions retained.

The Web lane independently decoded and hash-checked all PNGs and ran its unchanged Git provenance verifier successfully against authentic SHA-matched commit/tree/path/blob objects. The minimal object store intentionally lacks unrelated contents and parent history, and is not evidence of full checkout or ancestry completeness. No replace refs or changed commit were used.

These manifests remain coverageComplete=false with explicit NOT_RUN. Counts are captured states, not claims of additional unique physical screens. No equivalent actual HTML full-domain capture or final W3/F1/F2 acceptance is implied. PNG bytes are preserved locally; this checkpoint publishes identity/technical metadata only, not image delivery or a claim all renders are committed. Manifest paths are relative filenames within each capture output directory.

Reproduce from the exact source commit with required original source inputs installed, keeping separate output directories:

```sh
SMC_UI_TEST_FILE=tests/host/ui_screen_inventory.c SMC_UI_OUT=/tmp/fwui-af56-ordinary sh tests/host/smc_ui_test.sh
SMC_UI_TEST_FILE=tests/host/ui_overlay_inventory.c SMC_UI_OUT=/tmp/fwui-af56-overlays sh tests/host/smc_ui_test.sh
```

Validate generated file names, 240x240 dimensions and SHA256 against these manifests. Generated host/platform context and complete state coverage remain separate qualification requirements; these partial captures are not a final firmware build or device test.
