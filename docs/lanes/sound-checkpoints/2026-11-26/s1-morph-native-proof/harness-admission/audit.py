"""Read exact existing guarded run logs and output first observable admission differences."""
from pathlib import Path
import json,re,sys,hashlib
for arg in sys.argv[1:]:
 m,alias,rc=arg.split(':',2); p=Path(rc)
 run=p.read_text().splitlines()
 top=next(r for r in run if r.startswith('/tmp/')); d=Path(top)
 pcmp=(d/'audio.s16le').read_bytes()
 owner=json.loads((d/'owner.json').read_text())
 ink=(d/'scene.keys').read_text().strip().split(',')
 def times(pattern):
  return [int(x.group(1)) for l in (d/'run.log').read_text().splitlines()
          if (x:=re.search(pattern,l))]
 expected=lambda o:o['bank']==2 and o['status']==1 and o['bankB']==0 and o['statusB']==1 and o['arp']==1 and o['cutoff']==115
 first_event=int(ink[0].split(':')[0])
 counts={
  'model':m,'run':alias,'pcm_sha256':hashlib.sha256(pcmp).hexdigest(),
  'scene_sha256':hashlib.sha256((d/'scene.keys').read_bytes()).hexdigest(),'events':len(ink),
  'first_external_event_ms':first_event,
  'usb_enumerated_ms':times(r'\[\s*(\d+) ms\] usb: Configured'),
  'hello_ack_ms_seq7':times(r'\[\s*(\d+) ms\] usb in:\s+f0 7d 44 4e 03 40 07'),
  'nominal_owner_request_ms':[2060,2900,4100,5650,7800],
  'first_success_owner_get_ms':next((o['ms'] for o in owner['owners'] if expected(o)),None),
  'accepted_owner_get_seq':[o['seq'] for o in owner['owners'] if expected(o)],
  'prezero_settled_good':sum(o['ms']<first_event and expected(o) for o in owner['owners']),
  'first_pcm_nonzero_frame':next((i//4 for i in range(0,len(pcmp),2) if pcmp[i:i+2]!=b'\0\0'),None),
  'first_consumed_dsp_event_frame':None,'first_late_dsp_half_index':None}
 print(json.dumps(counts))
