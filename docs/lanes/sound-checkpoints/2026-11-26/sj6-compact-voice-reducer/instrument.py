"""Add read-only PCM measurement taps immediately before the compact reducer."""
from pathlib import Path
import hashlib
import sys
a=Path(sys.argv[1]).read_bytes()
assert hashlib.sha256(a).hexdigest()=="bd5a59c1b46fdf65f7d4e8eb0d9780fcbf21841906d00a672309a4ee20532f76"
match=b"        left[k]=0; right[k]=0;\n        for (unsigned l=0;l<DNBX_LAYERS;l++) {\n"
assert a.count(match)==1
x=a.replace(match,b"        extern float reducer_tap[2][DNBX_MAX_FRAMES];\n        reducer_tap[0][k]=mix[0]; reducer_tap[1][k]=mix[1];\n"+match)
with Path(sys.argv[2]).open("xb") as out: out.write(x)
