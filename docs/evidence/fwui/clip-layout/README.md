# Clip-key source layout guard after rule21

The previous static check required PLAY KEYS and the knob/arrow explainer removed by the reviewed rule21 correction. Replace those positive caption requirements with absence guards, retaining actual held-key lighting, selected fields, stage ON labels and five field rows/keybed bounds. Duplicate source anchors now reject.

Current7 source checks PASS. Fixture suite7 total cases PASS: one healthy and six negative cases (lighting, overlap, bounds, selection, removed caption and ambiguous extraction). Independent review replayed both against exact integration4e3b main/header. No production edit or all-state raster/geometry claim.
