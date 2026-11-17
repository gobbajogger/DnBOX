# SPDX-License-Identifier: GPL-3.0-only
"""Synthetic command-contract tests, never visual acceptance evidence."""
import json,tempfile,unittest,subprocess
from pathlib import Path
from unittest.mock import patch
from full_geometry import run,validate_audits
from screen_walk import expected_inventory
EXPECTED=len(expected_inventory())

class FullGeometryTests(unittest.TestCase):
 def exercise(self,status='PASS',count=EXPECTED,code=0,write=True):
  with tempfile.TemporaryDirectory() as d:
   work=Path(d)/'source';out=Path(d)/'geometry';work.mkdir()
   def command(argv,**kwargs):
    self.assertEqual(argv[1],str(work/'tests/ui_geometry/run.py'))
    self.assertEqual(argv[2:],['--source',str(work),'--output',str(out)])
    if write:
     out.mkdir();(out/'result.json').write_text(json.dumps({'status':status,'screens':count}))
    return subprocess.CompletedProcess(argv,code)
   with patch('full_geometry.subprocess.run',side_effect=command):return run(work,out)
 def test_actual_geometry_command_required(self):self.assertEqual(self.exercise()['screens'],EXPECTED)
 def test_fail_closed(self):
  for args in [dict(status='FAIL'),dict(status='NOT_RUN'),dict(count=0),dict(count=EXPECTED-1),dict(code=1),dict(write=False)]:
   with self.subTest(args=args),self.assertRaises((ValueError,OSError,subprocess.SubprocessError)):self.exercise(**args)
 def test_judge_cannot_skip_geometry_failure(self):
  import judge,sys
  with tempfile.TemporaryDirectory() as d:
   out=Path(d)/'out'
   with patch.object(sys,'argv',['judge.py','--output',str(out)]), \
        patch('judge.subprocess.check_output',return_value=d+'\n'), \
        patch('judge.subprocess.run',return_value=subprocess.CompletedProcess([],0,stdout=b'')), \
        patch('judge.full_geometry',side_effect=ValueError('geometry negative')) as required:
    self.assertEqual(judge.main(),1)
    required.assert_called_once()
    self.assertEqual(json.loads((out/'judgement.json').read_text())['status'],'FAIL')
class AuditCoverageTests(unittest.TestCase):
 def records(self):
  return [{'kind':'screen','id':name,'texts':3,'findings':0} for name in ('a','b')]+[{'kind':'complete','screens':2}]
 def check(self,rows):return validate_audits('\n'.join('AUDIT '+json.dumps(r) for r in rows),{'a','b'})
 def test_nonzero_actual_audit_each_state(self):self.assertEqual(self.check(self.records())['screens'],2)
 def test_missing_duplicate_zero_wrong_and_trailing_fail(self):
  rows=self.records()
  cases=[[],rows[1:],rows[:-1],rows[:1]+rows,rows+[rows[0]],
   [{**rows[0],'texts':0}]+rows[1:],[{**rows[0],'id':'wrong'}]+rows[1:],
   [{**rows[0],'texts':True}]+rows[1:]]
  for case in cases:
   with self.subTest(case=case),self.assertRaises(ValueError):self.check(case)
if __name__=='__main__':unittest.main()
