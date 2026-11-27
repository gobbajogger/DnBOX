from pathlib import Path
import hashlib, importlib.util, os, sys
import guarded_replay as g
r=Path.cwd()
w=Path((r/'s1-focus-rerun/emulator-boot.log').read_text().splitlines()[0])
assert w.is_relative_to('/tmp') and w.name.startswith('dnbox-emulator-only-')
d=r/'s1-focus-rerun/tree/joint-diagnostic-sealed'
ref={'elite':('414cb4f6e011c4026594d822fa455e28022d16212a232a7ed0261e8caf2ec105','748f3e1ba851ced902931e50a7dbfc38e00bd793d3ea901d32566e863223c0c8'), 'pro':('8da2d959a6031bafcbdf23946f8deee489c84930c5387410fc67e029bc63fffd','dd2e7f13bce547732cc5956553ae2d94a876b6097216ffbefc7bafcf23895b16')}
spec=importlib.util.spec_from_file_location('stock_template_reader',r/'fw/tools/fm1_rescue.py')
mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
scene=g.regular(r/'fw/experiments/sound-hardware-v1/strict-timing/heavy.keys')
assert hashlib.sha256(scene).hexdigest()==g.SCENE
ev=[(int(t),k) for t,k in (e.split(':',1) for e in scene.decode().strip().split(','))]
assert len(ev)==922
m=sys.argv[1]; off=int(sys.argv[2]); assert 0<=off<=5
elf=d/m/'never-install.elf'; pkg=r/'out'/f'DnBOX-0.0.27-SMK37-{m}-73749f55-test.fwsc'
_, sym, ram = g.project(mod,m,pkg,elf,ref[m][0])
seed=g.regular(w/f'{m}-s1-arp.bin')
assert g.sha(seed)==ref[m][1] and g.sha(g.regular(w/'fm1-live-ba83317a'))==g.EMU
result=g.replay(w,m,off,w/'fm1-live-ba83317a',pkg,ev,(seed,sym,ram))
print(result,flush=True)
