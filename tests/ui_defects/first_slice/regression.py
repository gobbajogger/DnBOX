#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Real render regression cases. Exit0 case clear; exit1 actual defect detected.
Same inspect_screen implementation on both sides; no modified expectations.
"""
import argparse,json
from pathlib import Path
from detectors import inspect_screen
from run import validate_render_receipts

CASES={
 'D01-inactive-multi-control-collision':('kit0-voice00-page01','overlaps'),
 'D02-sparse-808-panel':('kit1-voice05-page00','3'),
 'D03-unpainted-compact-captions':('kit0-voice07-page00','4'),
 'D04-distinct-ref-pitch-fork':('kit0-voice00-page00','2'),
 'D05-wip-TUTUNE-caption-overlap':('kit0-voice00-page01','text-overlaps'),
}

def write_receipt(path,record):
 with path.open('x') as output:json.dump(record,output,indent=2,sort_keys=True);output.write('\n')


def main():
 p=argparse.ArgumentParser();p.add_argument('--render-dir',type=Path,required=True);p.add_argument('--case',choices=CASES,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 _,raw=validate_render_receipts(a.render_dir.resolve());name,check=CASES[a.case]
 screen=next(s for s in raw['screens'] if s['id']==name);inspected=inspect_screen(screen)
 failures=([e for e in inspected['overlaps'] if e['kind'] in ('glyph','value')] if check=='overlaps' else [e for e in inspected['overlaps'] if e['kind']=='text'] if check=='text-overlaps' else inspected['checks'][check]['failures'])
 closure=json.loads((a.render_dir/'source-closure.json').read_text())['sha256']
 record={'schema':'dnbox-firmware-real-render-regression/1','defectId':a.case,'screen':name,'sourceClosureSha256':closure,
         'detector':check,'failures':failures,'exit':1 if failures else 0,
         'scope':'Actual source-backed host trace, not a selected mock. Composed historical source is labeled by its separate COMPOSITION.json.'}
 write_receipt(a.output,record);print(json.dumps({'case':a.case,'exit':record['exit'],'failures':len(failures)}))
 return record['exit']
if __name__=='__main__':raise SystemExit(main())
