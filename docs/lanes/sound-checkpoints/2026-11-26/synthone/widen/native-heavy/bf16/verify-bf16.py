import hashlib,json,subprocess,sys
from pathlib import Path
here=Path(__file__).resolve().parent
base=here.parent
prior=json.loads((here/'matched.json').read_text())
def sha(b):return hashlib.sha256(b).hexdigest()
def log_path(m,kind):
    name={'old':'baseline','h16':'loaded','bf16':'bf16'}[kind]
    return Path(next(x for x in (here/f'{m}-{name}.log').read_text().splitlines() if x.startswith('/tmp/dnbox-loaded-replay-')))
def seed_for(m,expected):
    for file in Path('/tmp').glob(f'dnbox-emulator-only-*/{m}-widen-room.bin'):
        if sha(file.read_bytes())==expected:return file
    raise RuntimeError('exact guard-sealed seed not found')
wanted=sys.argv[1:] or list(prior)
assert set(wanted).issubset(prior) and len(set(wanted))==len(wanted)
result={m:prior[m] for m in wanted}
for m,p in result.items():
    shot=log_path(m,'bf16')
    model,offset,elfhash,seedhash=next(x.split() for x in (here/'bf16.log').read_text().splitlines() if x.startswith(f'{m} 0 '))
    assert model==m and offset=='0'
    elf=base/f's1-organ-wet-recovery/native-widen-bf16/tree/widen-bf16-native/{m}/never-install.elf'
    assert sha(elf.read_bytes())==elfhash
    seed=seed_for(m,seedhash)
    candidate=seed.read_bytes()
    assert (shot/'disposable.bin').read_bytes()==candidate
    assert sha((shot/'scene.keys').read_bytes())==p['identical_scene_sha256']
    for block,expected in p['identical_project_sectors'].items():
        at=0x9d000+8192*int(block)
        assert sha(candidate[at:at+8192])==expected
    assert candidate[0xaf000:0xaf010]==b'SMK37DATAX0X\0\0\0\0'
    actual=json.loads((shot/'result.json').read_text())
    wit=json.loads(subprocess.check_output([sys.executable,str(here/'witness.py'),str(elf),str(shot),str(seed)],text=True))
    assert wit['target']==1 and wit['nor_changes']==0
    (here/f'{m}-bf16-emulator.raw.log').write_text((shot/'run.log').read_text())
    (here/f'{m}-bf16-witness.json').write_text(json.dumps(wit,indent=2)+'\n')
    p.update(bf16=actual,bf16_elf=elfhash,bf16_seed=seedhash,bf16_witness=wit,
        bf16_minus_h16_us=actual['worst_us']-p['trial']['worst_us'],
        bf16_minus_h16_xruns=actual['xruns']-p['trial']['xruns'],
        bf16_minus_old_us=actual['worst_us']-p['baseline']['worst_us'],
        bf16_minus_old_xruns=actual['xruns']-p['baseline']['xruns'])
print(json.dumps(result,indent=2))
