"""Validate independent receipts against both exact joined model links."""
import json, re
from pathlib import Path
p=Path(__file__).resolve().parent
links={}
for line in (p/'combined-link.receipt').read_text().splitlines():
 a=re.fullmatch(r'guarded (elite|pro) XIP (\d+) free (\d+) pool usable (\d+) sealed asset 55307 header [0-9a-f]{64} ELF ([0-9a-f]{64})',line)
 assert a, line
 model,used,free,pool,h=a.groups()
 assert int(used)+int(free)==552672 and int(free)>0 and int(pool)==536
 links[model]=h
assert len(links)==2
for model,h in links.items():
 for owner in (0,1):
  for case in ('gate','voice'):
   r=json.loads((p/f'combined-{case}-{model}-{owner}.json').read_text())
   assert r['elf_sha256']==h and r['model']==model and r['owner']==owner and r['flash_unchanged'] and r['audio_has_nonzero_pcm']
   a,b=r['state'][owner],r['state'][1-owner]
   assert a['arpKind']==b['arpKind']==3
   if case=='gate':
    assert a['gateCfg']==47 and a['latchTicks']==b['gateTicks']==32514048 and a['gateTicks']==30563205 and a['gateOpen']==a['noteCount']==1
    assert b['gateCfg']==50 and b['noteCount']==b['latchTicks']==0 and r['halves']==327
   else:
    assert a['mono']==a['legato']==a['voice0gate']==1 and a['voice0note']==54 and a['glideOffset']<-.01
    assert a['glide']>(63+9*owner) and a['raw']==[131+4*owner,a['glide']]
    assert b['mono']==b['legato']==b['voice0gate']==0 and b['glide']==63+9*(1-owner) and b['raw']==[128+4*(1-owner),b['glide']]
    assert r['revision']==112 and r['ui'][:2]==[1,owner+2] and r['ui'][13+owner]==13 and r['halves']==415
   print(model,owner,case,r['halves'],r['max_us'],r['raw_late'])
print('PASS both exact joined ELFs; 8 independent model/owner/feature shots, all audible, no simulated flash differences')
