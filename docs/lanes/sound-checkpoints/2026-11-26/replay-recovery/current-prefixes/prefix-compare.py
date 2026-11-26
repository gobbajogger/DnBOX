#!/usr/bin/env python3
"""Read sealed emulator replays only. Endpoint planner fields are not worst-half fields."""
import json,re,struct,subprocess,sys,hashlib
from pathlib import Path
elf,ref,shot=map(Path,sys.argv[1:4])
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def pcm(p): return (p/'audio.s16le').read_bytes()
def usb(p):
    return [(int(m.group(1)),m.group(2),m.group(3).split()) for l in (p/'run.log').read_text().splitlines()
            if (m:=re.match(r'\[\s*(\d+) ms\] usb (in|out):\s+(.*)$',l))]
def scene(p): return sha(p/'scene.keys')
def flash(p): return sha(p/'disposable.bin')
assert scene(ref)==scene(shot) and flash(ref)==flash(shot)
def snap(path):
    syms={}
    expected={'x0x_dbg':52,'eng_plan':1,'eng_c1_mask':1,'eng_c1_next':1,'eng_job_arg':4,'eng_job_cost':20,'eng_808_split':1,'eng_stage':4}
    for line in subprocess.check_output(['nm','-S',str(elf)],stderr=subprocess.DEVNULL,text=True).splitlines():
        f=line.split()
        if len(f)==4 and f[3] in expected:
            assert f[3] not in syms and int(f[1],16)==expected[f[3]],line
            syms[f[3]]=int(f[0],16)-0x1c00000
    assert set(syms)==set(expected)
    ram=(path/'ram.bin').read_bytes()
    fields={k: list(struct.unpack_from('<' + ('B' if size==1 else 'I'*(size//4)),ram,syms[k]))
            for k,size in expected.items()}
    dbg=fields.pop('x0x_dbg'); assert dbg[0]==0x44424731
    return {'half_count':dbg[1],'worst_us':dbg[2],'in_audio':bool(dbg[4]),'late':dbg[5],'last_us':dbg[8],
            'planner_endpoint':fields}
base,short=pcm(ref),pcm(shot)
assert len(short)<=len(base) and len(short)%4==0
# Discard at most one 512-frame DMA half only at the terminus, never in the interior.
usable=max(0,len(short)-512*4)
refusb,shortusb=usb(ref),usb(shot)
short_ms=max((t for t,_,_ in shortusb),default=0)
assert short_ms>0
edges=[min(len(base),len(short))//4,usable//4]
print(json.dumps({'scope':'Copy-to-copy exact sealed replay; planner fields describe only capture endpoint.',
  'elf_sha256':sha(elf),'shared_virtual_nor_sha256':flash(ref),'shared_scene_sha256':scene(ref),
  'pcm_frames_full_and_terminal_half_trimmed':edges,
  'pcm_equal_full_prefix':base[:len(short)]==short,
  'pcm_equal_before_terminal_half':base[:usable]==short[:usable],
  'usb_equal_entire_observed_prefix':shortusb==[item for item in refusb if item[0]<=short_ms],
  'short_last_midi_ms':short_ms,'full_endpoint':snap(ref),'short_endpoint':snap(shot)},indent=2))
