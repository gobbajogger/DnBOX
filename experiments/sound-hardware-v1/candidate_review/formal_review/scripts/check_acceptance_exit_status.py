#!/usr/bin/env python3
"""Probe verifier orchestration with synthetic gates; never execute DSP or a compiler.

The indexed verifier is copied unchanged. A subprocess imports it, replaces only
expensive leaf operations, and calls its real main/aggregation/output/exit logic.
This is a negative control of process status, not an audio acceptance rerun.
"""
import argparse
import hashlib
import json
import pathlib
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent
REL = 'experiments/sound-hardware-v1/candidate_review/verify_candidate.py'

RUNNER = r'''
import importlib.util, pathlib, subprocess, sys
script=pathlib.Path(sys.argv[1]); root=pathlib.Path(sys.argv[2]); passed=sys.argv[3]=='true'
spec=importlib.util.spec_from_file_location('reviewed_candidate_verifier',script)
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
m.HERE=root
m.BASE=root
m.bind=lambda p: None
m.Api=lambda d: None
m.subprocess.run=lambda args,**kw: subprocess.CompletedProcess(args,0,'stubbed compiler\n','')
m.subprocess.check_output=lambda args,**kw:'stubbed compiler (no compilation)\n'
m.envelope_tests=lambda api,out:[{'pass':passed,'ratio_crosscheck_pass':True,'negative_wrong_time_grid':{'pass':False},'negative_linear_envelope':{'pass':False}}]
m.velocity_tests=lambda api:{'linear_pass':True,'negative_squared_rejected':True}
m.lifecycle_tests=lambda api:{'synthetic':{'pass':True}}
m.error_tests=lambda api:{'all_pass':True}
m.finite_tests=lambda api:{'all_pass':True}
m.long_duration_tests=lambda api:{'pass':True}
sys.argv=[str(script),'--label','forced_'+('true' if passed else 'false'),'--source',str(root/'source')]
m.main()
'''

def sha(data):
    return hashlib.sha256(data).hexdigest()

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--package',type=pathlib.Path,required=True)
    p.add_argument('--output',type=pathlib.Path,required=True)
    p.add_argument('--source-script',type=pathlib.Path)
    p.add_argument('--false-exit',type=int,choices=(0,1),default=1,
                   help='Use 0 only when reproducing the pinned historical failure')
    args=p.parse_args()
    package=args.package.expanduser().resolve()
    out=args.output.expanduser().resolve()
    out.mkdir()
    source=args.source_script.read_bytes() if args.source_script else (package/'candidate_review/verify_candidate.py').read_bytes()
    copied=out/'verify_candidate.py'; copied.write_bytes(source)
    runner=out/'synthetic_gate_driver.py';runner.write_text(RUNNER)
    fake=out/'source';fake.mkdir()
    for name in ('dnbx_core.c','dnbx_core.h','fastmath.h'):
        (fake/name).write_text('Synthetic input: this file is hashed/copied, never compiled.\n')
    declaration=(package/'candidate_review/ACCEPTANCE-DECLARATION.json').read_bytes()
    (out/'ACCEPTANCE-DECLARATION.json').write_bytes(declaration)
    for rel in json.loads(declaration)['reference_hashes']:
        p=out/rel;p.parent.mkdir(parents=True,exist_ok=True)
        p.write_bytes((package/rel).read_bytes())
    cases=[]
    for passed in (False,True):
        cmd=[sys.executable,str(runner),str(copied),str(out),str(passed).lower()]
        cp=subprocess.run(cmd,capture_output=True,text=True)
        name='forced_'+str(passed).lower()
        (out/(name+'.stdout.log')).write_text(cp.stdout)
        (out/(name+'.stderr.log')).write_text(cp.stderr)
        receipt=json.loads((out/name/'RESULT.json').read_text())
        expected_exit=0 if passed else args.false_exit
        cases.append({'name':name,'command':cmd,'exit_code':cp.returncode,
                      'scoped_acceptance_pass':receipt['scoped_acceptance_pass'],
                      'receipt_written':True,'expected_exit_code':expected_exit,
                      'pass':cp.returncode==expected_exit and receipt['scoped_acceptance_pass']==passed})
    result={'source_sha256':sha(source),'source_path':str(args.source_script) if args.source_script else REL,
            'purpose':'Synthetic harness process-status negative/positive controls only; no DSP/compiler execution.',
            'cases':cases,'all_pass':all(x['pass'] for x in cases)}
    (out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))
    if not result['all_pass']:raise SystemExit(1)

if __name__=='__main__':main()
