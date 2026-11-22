#!/usr/bin/env python3
"""Apply the 0d09572a capture-health check to retained raw or losslessly compressed pairs."""
from argparse import ArgumentParser
import ast
import gzip
from pathlib import Path
import hashlib
import json
import struct
import subprocess

p=ArgumentParser(description=__doc__)
p.add_argument("heavy",type=Path, help="selected-native/heavy.py from 0d09572a or a compatible later checkout")
p.add_argument("build",type=Path)
p.add_argument("shots",type=Path)
a=p.parse_args()
asts=ast.parse(a.heavy.read_text())
functions=[f for f in asts.body if isinstance(f,ast.FunctionDef) and f.name in ("fault_snapshot","capture_health")]
assert len(functions)==2
env={"struct":struct}
exec(compile(ast.Module(body=functions,type_ignores=[]),str(a.heavy),"exec"),env)
def captured(path):
    return path.read_bytes() if path.is_file() else gzip.decompress(path.with_suffix(path.suffix+'.gz').read_bytes())
result={}
for receipt in sorted(a.shots.glob('*/heavy*/results.json')):
    model=receipt.parents[1].name; row=json.loads(receipt.read_text())
    elf=a.build/model/'x0x.elf'
    assert row['elf_sha256']==hashlib.sha256(elf.read_bytes()).hexdigest()
    nm=subprocess.check_output(['nm','-n',elf],text=True,stderr=subprocess.DEVNULL)
    syms={r[2]:int(r[0],16) for line in nm.splitlines() if len(r:=line.split())==3}
    snapshots=[env['fault_snapshot'](captured(receipt.parent/k/'ram.bin'),syms) for k in ['boot','heavy']]
    health=env['capture_health'](row['boot'],row['shot'],*snapshots)
    result[model+'/'+receipt.parent.name]={'elf_sha256':row['elf_sha256'],'timing_ok':row['timing_ok'],
              'max_us':row['shot']['max_us'], 'late':row['shot']['raw_late'],'xruns':row['shot']['audio_xruns'], 'capture_health':health}
print(json.dumps(result,indent=2))
