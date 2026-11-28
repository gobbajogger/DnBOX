"""Compare native raw stereo int16 frames without shifting either capture."""
import json
import struct
import sys
from pathlib import Path

a, b = [Path(name).read_bytes() for name in sys.argv[1:]]
assert len(a) == len(b)
pairs = zip(struct.iter_unpack("<I", a), struct.iter_unpack("<I", b))
diff = [i for i, (x, y) in enumerate(pairs) if x != y]
n = len(a) // 4
print(json.dumps(dict(frames=n, different_frames=len(diff),
                      first_difference=diff[0] if diff else None,
                      last_difference=diff[-1] if diff else None,
                      exact_suffix_frames=n-diff[-1]-1 if diff else n)))
