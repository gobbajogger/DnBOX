"""Executed current source Main/Sub stereo marker receiver; no sample inputs."""
from pathlib import Path
exec(compile(Path('sj6-modern-owner/probe.py').read_text().split('cases=[trial(')[0],
             'verified current original source schedule','exec'))
fn=Path('sj6-modern-owner/probe.py').read_text().split('def trial(',1)[1].split('cases=[trial(')[0]
a=' for k in (48,52,55):e.call(0x100389c84,d.dsp,k,86)'
assert fn.count(a)==1
fn=fn.replace(a,''' for j,k in enumerate((48,52,55)):
  if variant=='staggered' and j:go(33075+j*80)
  e.call(0x100389c84,d.dsp,k,86)''')
exec(compile('def alternate('+fn,'independent native marker same callback projection','exec'))
actual=[]
for label,lead in [('held',0),('middle_release',11025),('long_release',22050),('staggered',11025)]:
    m=alternate('oscSampler1',lead,'staggered' if label=='staggered' else 'default')
    s=alternate('sampler1',lead,'staggered' if label=='staggered' else 'default')
    assert m['commit']==s['commit'] and len(m['commit'])==1
    u=m['commit'][0][0]
    assert s['initialized_at_commit']==[[u,0.,True,0.]]
    rows=lambda v:[(r[0],r[1],r[7]) for r in v['prep']]
    assert rows(m)==rows(s)
    actual.append(dict(case=label,lead=lead,commit=u,compared_amp_words=len(m['prep']),
                       independent_sample_reset_exact=True))
assert len({v['commit'] for v in actual})>=2
actual_cfg=cfg['sampler_control_writes']
assert [dict(actual_cfg['oscSampler1'])[i] for i in range(10,16)]==[dict(actual_cfg['sampler1'])[i] for i in range(10,16)]
print(json.dumps(dict(cases=actual,scope='Actual current independent Main and Sub receivers under same projected AMP slots10–15 and co-timed receipt events; not proof of firmware compact envelope or current app ingress.'),indent=2))
