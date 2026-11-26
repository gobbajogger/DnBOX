from pathlib import Path
import json
p=Path(__file__).resolve().parent
off=json.loads((p.parents[1]/'strict-warm-j60/verify/summary.json').read_text())['result']
on=[json.loads((p/f'{m}-busy.json').read_text()) for m in ('elite','pro')]
assert on[0]['scene']==on[1]['scene']==off[0]['tape']==off[1]['tape']
assert on[0]['project_input_records']==on[1]['project_input_records']==off[0]['project_input_records']==off[1]['project_input_records']
assert all('verified: byte-identical disposable virtual NOR' in (p/f'{m}-sha256.txt').with_name(f'{m}.log').read_text() for m in ('elite','pro'))
out=[]
for a,b in zip(off,on):
    assert a['model']==b['model']
    v=a['raw_result']
    out.append(dict(model=a['model'],off=dict(elf=a['elf'],max_us=v['worst_us'],actual_xruns=v['xruns'],
      first_worst_half_ordinal_available=False),on=b))
print(json.dumps(dict(result=out,independent_inputs=dict(external_tape=on[0]['scene'],matched_seed_project_records=6,
    virtual_NOR_before_after_exact=True),restriction="Separate marked observations; no verified producer anchor or active SuperJ6 owner. Do not attribute difference from OFF to any one source/engine, and do not claim timing pass."),indent=2))
