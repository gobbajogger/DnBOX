"""Sealed disposable emulator projection only; no updater, flash writer or package."""
import importlib.util, json, runpy, sys
from pathlib import Path
d=Path(__file__).resolve().parent; r=d.parent.parent
sys.path.insert(0,str(r/'restored-harness-inspect/helpers'))
import guarded_replay as g
pins={'elite': {'aa2cacb33abf17bf35b287db214f82185f0bbc8a226f2daad63fb814ce66ac69','16253badc055ad7bcae4a43ad0e091df445e3e4c3448ae900fa775529a2a917d'},
      'pro': {'34537d270385218c6eddc08ccf1d752a558a69c39385db7746c3867c594beb86','4769f4e063ac3111b78c2feff5fddd10686a37ce8c717caabd11f38f00d48496'}}
old={'elite':'d9953d21980f6ae7dc26296c8b513618f11389db87beb54430ff7c2d3377ff31',
     'pro':'487e879c06aa19fc67ed809f6cbfa15fc1d80759b90326c2e0c3cc39b3271600'}
guarded_project=g.project
def project(loader,model,pkg,elf,expected):
    b,sym,sec=g.symbols_and_sections(elf)
    g.ensure(g.sha(b)==expected and expected in pins[model],'exact paired ELF pin')
    base,syms,ram=guarded_project(loader,model,pkg,elf,expected)
    payload=g.section(sec,'.lab_morph',0x020cd000,0x5000)
    g.ensure(len(payload)==20372 and g.sha(payload)=='fc111ab31296ad3897716dd98578372ef9063db5f4cb9c5e9a34e2c2269065bd','morph code/data pin')
    at=0xd1000
    g.ensure(base[at:at+0x5000]==b'\xff'*0x5000,'receiver nonempty')
    return base[:at]+payload+base[at+len(payload):],syms,ram
g.project=project
verb,model,elf,expected,seed=sys.argv[1:];seed=Path(seed)
g.ensure(seed.parent.parent == d and seed.parent.name.startswith('dnbox-emulator-only-'),'only ephemeral emulator-only envelope')
if verb=='make':
    p=r/'out'/f'DnBOX-0.0.27-SMK37-{model}-73749f55-test.fwsc'
    spec=importlib.util.spec_from_file_location('read_only_seed',r/'fw/tools/fm1_rescue.py')
    l=importlib.util.module_from_spec(spec);spec.loader.exec_module(l)
    base,_,_=project(l,model,p,Path(elf),expected)
    authored=g.regular(r/'recovered-sound/fresh-h16-replay/files/sealed'/f'{model}-h16-virtual-nor.bin')
    g.ensure(g.sha(authored)==old[model],'authored project pin')
    loaded=bytearray(base)
    for i in (0,1,2,3,5,6):
        o=0x9d000+8192*i;n=g.u32(authored,o+12)
        g.ensure(4<=n<=3840,'project payload length')
        loaded[o:o+32]=authored[o:o+32]; loaded[o+256:o+256+n]=authored[o+256:o+256+n]
    loaded[0xaf000:0xaf010]=authored[0xaf000:0xaf010]
    with seed.open('xb') as f:f.write(loaded)
    print(g.sha(loaded))
elif verb=='check':
    sys.argv=[sys.argv[0],model,elf,expected,str(seed)]
    runpy.run_path(str(r/'widen-native-verification/check-seed.py'),run_name='__main__')
else:
    raise ValueError(verb)
