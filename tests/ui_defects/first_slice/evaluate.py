#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Evaluate saved authentic host traces once using the exact final detector.
Does not rerender/retry the host. Prior output is never overwritten.
"""
import argparse,json,struct
from pathlib import Path
from detectors import inspect_screen
from run import digest,write,HERE,validate_screens,validate_render_receipts


def main():
 p=argparse.ArgumentParser();p.add_argument('--render-dir',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 if a.output.exists():raise ValueError('output must be new; retain prior evaluations')
 root=a.render_dir.resolve();old,raw=validate_render_receipts(root)
 validate_screens(raw.get('screens',[]),raw.get('complete'))
 screens=[inspect_screen(s) for s in raw['screens']];checks={}
 for i in range(1,25):
  if i>4:checks[str(i)]={'status':'NOT_RUN','evidence':'Outside implemented first slice.'};continue
  failures=[{'screen':s['id'],**f} for s in screens for f in s['checks'][str(i)]['failures']]
  status='FAIL' if failures else ('NOT_RUN' if any(s['checks'][str(i)]['status']!='PASS' for s in screens) else 'PASS')
  checks[str(i)]={'status':status,'failures':failures,'evidence':'Actual traced renderer + final framebuffer font-mask proof.'}
 overlaps=[{'screen':s['id'],**f} for s in screens for f in s['overlaps']]
 for s in screens:
  path=root/'host'/(s['id']+'.png');data=path.read_bytes();width,height=struct.unpack('>II',data[16:24])
  s['png']={'path':str(path),'sha256':digest(path),'width':width,'height':height,'sourceClosureSha256':old['sourceClosureSha256'],'kind':'png'}
 artifacts=[{'path':str(root/name),'sha256':digest(root/name),'sourceClosureSha256':old['sourceClosureSha256'],'kind':'json' if name.endswith('.json') else 'log'} for name in ('source-closure.json','instrumentation.json','run.json','traces.json','renderer-first-attempt.log')]
 report={**old,'checks':checks,'screens':screens,'overlaps':overlaps,'detectorExit':1 if overlaps or any(c['status']=='FAIL' for c in checks.values()) else 0,
   'evaluation':{'detectorSha256':digest(HERE/'detectors.py'),'traceSha256':digest(root/'traces.json'),'originalReportSha256':digest(root/'report.json'),
                 'method':'Unchanged saved render traces; no host rerun. Fail-closed static45-screen inventory and actual required structure verified; actual final-font text bounds included. Prior evaluations preserved.'},
   'evidenceArtifacts':artifacts,'coverageComplete':False}
 write(a.output,report)
 print(json.dumps({'sourceClosureSha256':report['sourceClosureSha256'],'exit':report['detectorExit'],'overlaps':len(overlaps),
  'checks':{i:{'status':c['status'],'failures':len(c.get('failures',[]))} for i,c in checks.items() if int(i)<=4}}))
 return report['detectorExit']
if __name__=='__main__':raise SystemExit(main())
