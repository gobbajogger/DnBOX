#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Bounded native-annotation differential. Original reference files stay intact."""
from pathlib import Path
import argparse, hashlib, os, shlex, subprocess, tempfile
ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'tests/host/dnr2_reference'
PINS={'remote_core.c': '4f7fae3087e3477d8bf8a65f77c3189dddddd3e1d34b8c264b3b1017578f97d4', 'remote_wire.h': 'ad878321249e107d532c8f69d48af1d702e397c990a43f47c4e0b5d1fe085cd5', 'remote_core.h': 'ada39c8d12999c92debf50c787da66c0708122b8cfe486005b1ff1559d6ba7ae', 'remote_wire.c': '81b4a8729cab7af75bb8495f528d9979d0fc02abbedfba05aaac3f0dae659390'}
p=argparse.ArgumentParser();p.add_argument('--mode',choices=['normal','ubsan','asan','all'],default='all');args=p.parse_args()
cc=shlex.split(os.environ.get('CC','cc'))
captured={name:(SRC/name).read_bytes() for name in PINS}
for name,digest in PINS.items():
    assert hashlib.sha256(captured[name]).hexdigest()==digest, 'Reference changed; re-review exact source before repinning'
harness=(ROOT/'tests/host/remote_native_annotations_test.c').read_bytes()
assert hashlib.sha256(harness).hexdigest()=='534760677554003111ac2f704ece8ce9063bee0e5cd55fd2adf9c6d1fec60dba', 'Differential fixture changed; review before repinning'
functions=['dnr2_init','dnr2_changed','dnr2_tick','dnr2_handle','dnr2_decode','dnr2_encode_command','dnr2_encode_reply','dnr2_receive']
prefix=['-D'+f+'=ref_'+f for f in functions]
with tempfile.TemporaryDirectory(prefix='dnbox-remote-annotations-') as temp:
    t=Path(temp); original_dir=t/'original'; original_dir.mkdir()
    for name,data in captured.items(): (original_dir/name).write_bytes(data)
    fixture=t/'differential.c'; fixture.write_bytes(harness)
    def verify_staged():
        for name,data in captured.items(): assert (original_dir/name).read_bytes()==data
        assert fixture.read_bytes()==harness
    for name,decls in [('remote_core.c',['static dnr2_reply base(','static dnr2_reply save(']),('remote_wire.c',['static uint32_t u7(','static void p7('])]:
        original=captured[name].decode();derived=original
        for decl in decls:
            assert derived.count(decl)==1
            derived=derived.replace(decl,decl.replace('static ','static __attribute__((noinline)) ',1))
        assert derived.replace('__attribute__((noinline)) ','')==original
        (t/name).write_text(derived)
    derived_bytes={name:(t/name).read_bytes() for name in ('remote_core.c','remote_wire.c')}
    modes=['normal','ubsan','asan'] if args.mode=='all' else [args.mode]
    for mode in modes:
        verify_staged()
        for name,data in derived_bytes.items(): assert (t/name).read_bytes()==data
        flags=['-O2'] if mode=='normal' else ['-O1','-g','-fsanitize='+('undefined' if mode=='ubsan' else 'address,undefined'),'-fno-sanitize-recover=all','-fno-omit-frame-pointer']
        common=[*cc,'-std=c99','-Wall','-Wextra','-Werror',*flags,'-I'+str(original_dir)]
        objects=[]
        for name in ('remote_core.c','remote_wire.c'):
            obj=t/(name+'.o');subprocess.run([*common,*prefix,'-c',str(original_dir/name),'-o',str(obj)],check=True);objects.append(str(obj))
        exe=t/'differential'
        subprocess.run([*common,str(fixture),str(t/'remote_core.c'),str(t/'remote_wire.c'),*objects,'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
        verify_staged()
        for name,data in derived_bytes.items(): assert (t/name).read_bytes()==data
        print('PASS annotation differential mode='+mode,flush=True)
print('No native application, stack, latency, linked fit or endpoint admission is implied.')
