from pathlib import Path
import sys,json
p=Path(__file__).resolve().parent
root=p.parent.parent
sys.path.insert(0,str(root/'restored-harness-inspect/helpers'))
import guarded_replay as g
stg=Path(sys.argv[1])
pins={'elite':'39986b42b39fe3e300cdf8a7fb60996ef475580118c664f7efac4961d9d82918',
      'pro':'df5831a1a84f8b52198f9af9112ac44a97c12759743d710d4f2b35d215db4570'}
for m,want in pins.items():
    loc=stg/m; raw,sym,sec=g.symbols_and_sections(loc/'never-install.elf')
    assert g.sha(raw)==want
    h={}
    for n in ('lab_fdata','lab_slot3','lab_fittab','xdata','lab_morph'):
        t=g.regular(loc/f'{n}.bin')
        a,l,b=sec['.'+n]
        assert len(t)==l and t==b
        h[n]={'vma':hex(a),'bytes':l,'sha':g.sha(t)}
    print(json.dumps(dict(model=m,elf_sha=want,subregions_verified=h)))
