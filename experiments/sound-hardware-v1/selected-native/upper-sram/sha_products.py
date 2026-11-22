#!/usr/bin/env python3
"""Compare the supported FM-1 Python release build to its unchanged baseline."""
from argparse import ArgumentParser
from hashlib import sha256
import json
from pathlib import Path

p = ArgumentParser(description=__doc__)
p.add_argument('baseline',type=Path)
p.add_argument('candidate',type=Path)
args=p.parse_args()
result={}
for name in ('x0x.bin','x0x.fwsc','loader/ota.bin'):
    original=(args.baseline/name).read_bytes()
    assert (args.candidate/name).read_bytes()==original
    result[name]={'bytes':len(original),'sha256':sha256(original).hexdigest()}
print(json.dumps(result,indent=2))
