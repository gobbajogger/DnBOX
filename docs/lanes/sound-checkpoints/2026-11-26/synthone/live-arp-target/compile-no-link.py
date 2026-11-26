"""Run actual toolchain build, stop before any app/bootloader link or packaging."""
from tools import build as b
import os
from pathlib import Path
b.CFLAGS.extend(Path(os.environ["DNBOX_CONTROL_FLAGS"]).read_text().split())
real=b.tc
class EndCompile(Exception):pass
def tc(name,*args):
    if name=="pi32v2/bin/ld":
        print("compiled-controls",b.MODEL,flush=True)
        raise EndCompile
    return real(name,*args)
b.build_loader=lambda:None
b.tc=tc
try:
    b.main()
except EndCompile:
    pass
else:
    raise SystemExit("No native application-link interception")
