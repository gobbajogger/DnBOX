"""Native stock-region link only. Never package or run firmware."""
from tools import build as b
import os
from pathlib import Path

class DoneCompiling(Exception):
    pass

original_tc = b.tc
def compile_only(tool, *args):
    if tool == "pi32v2/bin/ld":
        original_tc(tool, *args[:-2], "-Map", b.OUT / "controls-fit.map", *args[-2:])
        print("stock-linked-controls", b.MODEL, flush=True)
        raise DoneCompiling
    return original_tc(tool, *args)

b.CFLAGS.extend(Path(os.environ["DNBOX_CONTROL_FLAGS"]).read_text().split())
b.ORIGINAL_SYNTH_OZ_UNITS |= {"chicago_selector", "chicago_arp_control",
                             "chicago_note_context", "miami_geometry",
                             "miami_fx_conversion"}
b.generate = lambda: None
b.build_loader = lambda: None
b.tc = compile_only
try:
    b.main()
except DoneCompiling:
    pass
