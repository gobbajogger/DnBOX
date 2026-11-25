"""Score the six fixed private references and compare to the prior headless rerender."""
import json,sys,numpy as np
from pathlib import Path
p=Path(__file__).resolve().parent
r=p.parent
sys.path.insert(0,str(r/'s1-controlled-ab'))
import measure
d=[]
profiles={row["program"]:row["preview_gain"] for row in json.loads((r/'s1-controlled-ab/calibration.json').read_text())}
for i,g in profiles.items():
    a=measure.wav(r/f'restored-s1-ab-private/corrected-originals/{i:03d}-original.wav')
    c=measure.floats(p/f'out/{i}.f32le')
    b=measure.floats(r/f's1-controlled-ab/recovered/{i}-wet.f32le')
    new=measure.margin(a,c*g);base=measure.margin(a,b*g)
    passes=lambda q:max([abs(u) for u in [*q['whole_dB'],q['note_body_dB'],q['last_1s_dB']]])<=1 and max(q['spectral_distance_db'])<=3
    d.append(dict(i=i,unchanged=np.array_equal(b,c),new=new,base=base,pass_level_and_spectral=passes(new)))
assert sum(row['unchanged'] for row in d)==5
assert next(row for row in d if row['i']==113)['pass_level_and_spectral']
print(json.dumps(d,indent=2))
