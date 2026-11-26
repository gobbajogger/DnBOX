from pathlib import Path
import json
p=Path(__file__).resolve().parent; root=p.parents[1]
occ=json.loads((root/'warm-j60-applied-busy-top/diagnostic/paired-report.json').read_text())['result']
cost=[json.loads((p/f'{m}-busy.json').read_text()) for m in ('elite','pro')]
assert cost[0]['scene']==cost[1]['scene']==occ[0]['on']['scene']==occ[1]['on']['scene']
assert cost[0]['project_input_records']==cost[1]['project_input_records']==occ[0]['on']['project_input_records']==occ[1]['on']['project_input_records']
assert all('verified: byte-identical disposable virtual NOR' in (p/f'{m}.log').read_text() for m in ('elite','pro'))
out=[]
for previous,new in zip(occ,cost):
 assert previous['model']==new['model']
 assert new['worst_actual_miss']['half_ordinal']==new['worst_half_elapsed_decomposition']['ordinal']
 qb=previous['on']
 out.append(dict(model=new['model'],unmarked_reference=previous['off'],occupancy_only_reference=dict(
     elf=qb['elf'],worst=qb['worst_actual_miss'],aggregate=qb['aggregate']),cost_recorder=new))
print(json.dumps(dict(result=out,paired_unchanged_event_tape=cost[0]['scene'],seed_record_exact_count=6,
    immutable_disposable_nor_each=True,scope="Two actual completed C0 runtime intervals sampled with existing clock, not nested IRQ-exclusive. No preflight/active SuperJ6/producer anchor, no causal inference across probes or release."),indent=2))
