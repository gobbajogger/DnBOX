# SPDX-License-Identifier: GPL-3.0-only
"""Run the full inventory's real geometry checks, not a saved receipt."""
import json,subprocess,sys
from pathlib import Path
from screen_walk import expected_inventory

def run(work,out):
    work=Path(work).resolve();out=Path(out).resolve()
    proc=subprocess.run([sys.executable,str(work/'tests/ui_geometry/run.py'),
                         '--source',str(work),'--output',str(out)],check=True)
    if proc.returncode:raise ValueError('full geometry renderer failed')
    report=json.loads((out/'result.json').read_text())
    if report.get('status')!='PASS' or type(report.get('screens')) is not int or report['screens']!=len(expected_inventory()):
        raise ValueError('full geometry incomplete or failed')
    return report

def validate_audits(log,expected=None):
    """Every frozen state must execute a nonempty opaque-text geometry audit."""
    expected=set(expected_inventory()) if expected is None else set(expected)
    rows={};complete=None
    for line in log.splitlines():
        if not line.startswith('AUDIT '):continue
        record=json.loads(line[6:])
        if not isinstance(record,dict) or complete is not None:raise ValueError('invalid or trailing geometry audit')
        if record.get('kind')=='complete':
            complete=record.get('screens')
            if type(complete) is not int:raise ValueError('invalid audit completion')
            continue
        name=record.get('id');texts=record.get('texts');findings=record.get('findings')
        if record.get('kind')!='screen' or not isinstance(name,str) or name in rows or type(texts) is not int or texts<=0 or type(findings) is not int or findings<0:
            raise ValueError('missing, duplicate or empty per-state geometry audit')
        rows[name]=record
    if not expected or set(rows)!=expected or complete!=len(expected):raise ValueError('geometry audit inventory incomplete')
    return {'screens':len(rows),'texts':sum(r['texts'] for r in rows.values()),'findings':sum(r['findings'] for r in rows.values())}
