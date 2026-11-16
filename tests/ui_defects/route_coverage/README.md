# Production dispatch coverage probe

This probe instruments an isolated source copy with counters, then runs actual
ordinary/global page construction and selected real help, confirmation, SMC,
readout, clock-wait and transfer-header paths. It changes no production branch
source. No screenshots are saved and no pixel/style acceptance is inferred.

```
python3 tests/ui_defects/route_coverage/run.py --source . --output /new/output
python3 -m unittest discover -s tests/ui_defects/route_coverage -p 'test_*.py' -v
```

The output directory must not exist. The complete copied input hash manifest,
first compile/run log and route report remain there. Missing/duplicated source
anchors, unexpected source mutations, compilation failure and incomplete counter
output abort the producer rather than manufacture a coverage result.

`OBSERVED` means only that the actual instrumented function or branch executed.
Counts include band passes, setup and repeated frames, not distinct screens.
The route list covers current main-body dispatch plus exceptional frame routing;
it is not every conditional inside every renderer, every header/knob variant,
engine/preset/control value, animation state or a semantically complete screen
inventory. A future added dispatch branch requires source review of this list.

The host fixture substitutes the actual hardware clip-grid and pad-calibration
renderers. Those rows remain `NOT_RUN`, including when the parent dispatch branch
was entered. The real waiting-screen code lives in ui.c and can be observed
without pretending the stub grid is a device render. No hardware is accessed.
`coverageComplete` is always false: dispatch coverage is a development diagnostic,
not F1/F2 completion or a replacement for independent visual review.
