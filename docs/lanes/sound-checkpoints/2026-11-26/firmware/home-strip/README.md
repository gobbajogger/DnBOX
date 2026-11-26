# Default Home 80+58 snapshot witness
On a disposable copy of the DnBOX tree, insert this before the unique `/* SONG: a table of bars` marker in `firmware/src/app/ui.c`:
```c
#if defined(X0X_HOST) && defined(DNBOX_HOME_STRIP_WITNESS)
#include "home-snapshot.inc"
#endif
```
Copy `home-snapshot.inc` next to that file and `home-test.c` to `tests/host/`. Run the source checkout's harness with `X0X_HOST_DEFS=-DDNBOX_HOME_STRIP_WITNESS=1 SMC_UI_OUT=/tmp/home-strip-output SMC_UI_TEST_FILE=tests/host/home-test.c sh tests/host/smc_ui_test.sh`.

The test binds actual default Home `draw_home` and actual RGB565 primitive/text and dirty-tile code. First DMA mutates live bank, save status and palette; no subsequent field read is allowed in the candidate. A stale/non-snapshot mode explicitly re-captures on physical strip two and must differ. Mock windows and asynchronous payload ownership are compared. This does not simulate target timing or real input dispatch; the explicit fake action is equivalent to changing those app-owned values at that boundary. Upper-memory canary checks prove this one path does not access rows 82–137. The active switch, device `cv_px` allocation, other pages, render-time changes from timer IRQ, and current integration/native testing are outside scope. Initial 82-row app pool saving is still hypothetical; subtract candidate snapshot/renderer costs, do not budget full gross if additional scratch remains live.
