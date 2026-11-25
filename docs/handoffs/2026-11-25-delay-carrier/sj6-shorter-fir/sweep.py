"""Bound the filter-cost tradeoff. Same two bands and ring; static 250 ms only."""
from pathlib import Path
import subprocess,json
p=Path(__file__).resolve().parent
base=p.parent/'sj6-delay-subband'
r={}
for n in (321,193,129,97,65,49,33):
    assert n%2==1
    source=(base/'subband.c').read_text().replace('N=321',f'N={n}')
    gen=(base/'filter_gen.py').read_text().replace('321',str(n))
    (p/'subband.c').write_text(source)
    (p/'filters.inc').write_bytes(subprocess.check_output(['python3','-c',gen]))
    subprocess.run(['cc','-O2','-std=c11','-ffp-contract=off','-fPIC','-shared','subband.c','-o','subband.so'],cwd=p,check=True)
    j=json.loads(subprocess.check_output(['python3','probe.py'],cwd=p))
    r[str(n)]={'state':j['history_bytes'],'max_nominal_macs_per_stereo_frame':8*n/16,'selected':[x for x in j['rows'] if x['mode']=='subband' and x['fb']==.499 and x['rep'] in (1,4) and x['f'] in (1000,3000,3400)]}
print(json.dumps(r,indent=2))
