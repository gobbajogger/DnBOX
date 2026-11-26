"""Decompose measured signed residual and levels; no additional PCM or rendering."""
from pathlib import Path
import json, math
p=Path(__file__).parent
s=json.loads((p/'result.json').read_text())
def cosine(m):
    aq=10**(m['level_db']/20)
    e=0 if m['signed_dbr'] is None else 10**(m['signed_dbr']/20)
    return (1+aq**2-e**2)/(2*aq)
d={}
for window,m in s['windows'].items():
    d[window]={}
    for kind in ['authored','dry','authored_minus_dry']:
        v=m[kind]
        d[window][kind]={'signed_correlation':[cosine(x) for x in v], 'signed_db_relative':[x['signed_dbr'] for x in v],
                         'relative_level_db':[x['level_db'] for x in v]}
    aa=m['authored_minus_dry'];dd=m['dry']
    d[window]['relative_wet_amplitude_gap_after_dividing_by_each_arms_dry_amplitude_db']=[a['level_db']-b['level_db'] for a,b in zip(aa,dd)]
print(json.dumps(d,indent=2))
