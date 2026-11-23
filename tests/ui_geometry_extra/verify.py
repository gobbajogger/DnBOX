#!/usr/bin/env python3
"""Fail closed on missing/empty extra-frame raster audits or any actual finding."""
import argparse,json
from pathlib import Path

def verify(log):
 rows=[]
 for line in log.read_text().splitlines():
  if line.startswith('F2_EXTRA '):rows.append(json.loads(line[9:]))
 if not rows:raise ValueError('no extra final-frame audits')
 if [r['frame'] for r in rows]!=list(range(len(rows))):raise ValueError('missing/duplicate extra-frame audit')
 for r in rows:
  if any(type(r[k])!=int or r[k]<0 for k in ['frame','labels','audited','findings']):raise ValueError('invalid count')
  if not r['id'] or r['scope'] not in ['DISPLAY_SLICE_ONLY','ACTUAL_HOST_COMPOSITION']:raise ValueError('invalid identity or scope')
  if r['audited']<1:raise ValueError('no opaque text audited for '+r['id'])
  if r['findings']:raise ValueError('final-frame text geometry finding: '+r['id'])
 return {'status':'PASS','frames':len(rows),'audited_texts':sum(r['audited'] for r in rows),'partial_display_slices':sum(r['scope']=='DISPLAY_SLICE_ONLY' for r in rows)}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('log',type=Path);a=p.parse_args()
 try:print(json.dumps(verify(a.log)))
 except (ValueError,KeyError,TypeError,json.JSONDecodeError) as e:print(json.dumps({'status':'FAIL','reason':str(e)}));raise SystemExit(1)
