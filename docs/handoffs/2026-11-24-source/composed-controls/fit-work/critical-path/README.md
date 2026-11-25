# Scheduler critical-path research checkpoint

This offline model accounts separately for the worker-side 808 BD+SD
predecessor, render/mix ordering and worker startup. The 808 body cost is
included exactly once. No firmware changes, timing wins, or release claims.
Run `python critical_path.py` for synthetic fixtures; feed measured,
wait-excluded durations for any real comparison. Do not replace the firmware
planner before measuring both models. The baseline source and results are
pinned at eae5e7455b578bf84df82f8dd2235872dc606802.

After the runner reset, the handoff files, JieLi inputs, exact emulator and
922-event scene were recovered. The pinned emulator SHA is
fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017
from the owner Library archive dot-gate-harness.zip; scene SHA is
d08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271.
The archive's gate-lock.sh is not the prior guarded ELF projection/overlay
wrapper. Reconstruct that virtual-only path and the exact dual-model joined
sources from the earlier Git handoffs before attempting another strict run.
Do not use stock-package results as evidence for the combined diagnostic.
