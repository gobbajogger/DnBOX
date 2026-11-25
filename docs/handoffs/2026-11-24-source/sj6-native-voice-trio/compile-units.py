# Disposable target compilation only; never enters linker/bootloader/package.
import sys
from pathlib import Path
from tools import build as b
names={'x0x.c','engine.c','original_runtime.c','project.c','dnbx_core.c','juno6.c'}
real=b.tc
class Stop(Exception): pass
def compile_only(tool,*args):
    if tool.endswith('/ld'): raise Stop
    if tool!='cc': return
    inputs={Path(str(p)).name for p in args}
    if inputs & names:
        print('UNIT', *(inputs & names),flush=True)
        real(tool,*args)
b.tc=compile_only
b.CFLAGS.extend(['-DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1','-DX0X_S1_MOD_RATE_PROBE=1','-DX0X_S1_BEND_RANGE_PROBE=1','-DX0X_SJ6_MONO32_PROBE=1'])
b.generate=lambda:None
b.build_loader=lambda:None
try:
    b.main()
except Stop:
    print('STOP before target link')
