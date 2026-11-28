# Experimental same-generation nonmodal publication within 82 rows

Base: viewport82-e00.patch. Apply exactly one staged82-* patch; no production
default or audio change. Top and bottom rasters are both prepared before the
first callback can service UI input. The later physical band is captured
first and encoded into the 6240-byte (13-row) unused tail of the one canvas;
the earlier band is drawn and sent normally. Existing LCD tile send order
and input callbacks remain. Restore later exact RGB565 pixels after the
last upper transfer finishes and send them without accessing live UI.

RLE1 (vertical repeats + solid runs + literals): stock frame A/B 139/139,
and 1800 ordinary nonmodal live-injection comparisons exact; peak 6128 B.
Flash 472 B free with only the initial +448 cold patch. Do not adopt.
RLE2 (only vertical/horizontal predictors): 744 B free with all cold
patches, but CAPACITY rejects dense V_SEQ; no partial publication. Reject.

LZ reuses the existing bounded project codec. Its dictionary is touched
only before upper DMA/first callback and only when PERF doesn't hold it;
the payload lives only in the canvas tail, never in the scratch that an
input callback can reuse. Stock SMK firmware compiles synthetic PERF off
(ui_perf_profile.h); instrumented PERF testing cannot yet use this viewport
path and must use the larger framebuffer diagnostic config instead.

LZ passed 3150 actual dense nonmodal frames with 3150 mutations after
first band publication, 104,328,000 exact RGB565 pixels. All 32 patterns
contain randomized drums and bass; seven views × five parts × six themes,
both pattern banks and HOME live toggle included. Peak 5366/6240; zero
rejects. This is not a capacity proof for every possible user project.
Overflow is checked before any pixel publication and retains previous
frame. Keep the bounded hold as diagnostic behavior, not a production
fallback for dense projects. Native loaded audio and worst foreground
latency still require measurement; no zero-dropout or installer claim.

Dense host fixture: use copied x0x_host_dense.c at host/ and run
modal-regression.py against the patched isolated checkout to generate
ui.modal-test.inc; build a host wrapper named from host/build_host.sh
replacing x0x_host.c with x0x_host_dense.c in the compile line. Set
X0X_HOST_DEFS='-DX0X_UI_SMALL_CANVAS=1 -DX0X_HOST_UI_SOURCE="../firmware/src/app/ui.modal-test.inc"',
X0X_SYNTHONE_PATCH=1, X0X_SUPERJ6_FACTORY=1. The fixture stores expected
frame before each injection; underlying stock reference comes from the
separate paired 139-frame no-mutation corpus.
