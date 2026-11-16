#!/usr/bin/env python3
"""Render exact committed firmware source, evaluate receipts, require complete coverage."""
import argparse,hashlib,importlib.util,json,pathlib,subprocess,sys,tarfile,tempfile

def unmet(report, profile_hash):
    reasons=[]
    if report.get('profileSha256')!=profile_hash:reasons.append('source profile mismatch')
    if report.get('run',{}).get('exit')!=0:reasons.append('renderer did not pass')
    if report.get('detectorExit')!=0:reasons.append('implemented detector failed')
    if report.get('coverageComplete') is not True:reasons.append('coverage incomplete')
    checks=report.get('checks',{})
    for i in range(1,25):
        if checks.get(str(i),{}).get('status')!='PASS':reasons.append('check'+str(i)+' not PASS')
    return reasons

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output-root',type=pathlib.Path,default=pathlib.Path('build/dnbox-gate'));a=ap.parse_args()
    root=pathlib.Path(subprocess.check_output(['git','rev-parse','--show-toplevel'],text=True).strip())
    def git(*args):return subprocess.check_output(['git','-C',str(root),*args],text=True).strip()
    if git('status','--porcelain','--untracked-files=no'):raise ValueError('committed source must be clean')
    sha=git('rev-parse','HEAD');tree=git('rev-parse','HEAD^{tree}')
    outroot=a.output_root.resolve();outroot.mkdir(parents=True,exist_ok=True)
    out=pathlib.Path(tempfile.mkdtemp(prefix='ui-'+sha[:8]+'-',dir=outroot));snapshot=out/'source';snapshot.mkdir()
    archive=out/'source.tar'
    with archive.open('wb') as stream:subprocess.run(['git','-C',str(root),'archive','--format=tar',sha],stdout=stream,check=True)
    with tarfile.open(archive) as tar:tar.extractall(snapshot,filter='data')
    # Git archive attributes must not omit or substitute any tracked input.
    records=subprocess.check_output(['git','-C',str(root),'ls-tree','-rz','--full-tree',sha]).split(b'\0')
    for record in records:
        if not record:continue
        head,name=record.split(b'\t',1);mode,kind,blob=head.decode().split();path=snapshot/name.decode()
        if kind!='blob':raise ValueError('unsupported non-blob source entry')
        raw=path.readlink().as_posix().encode() if mode=='120000' else path.read_bytes()
        actual=hashlib.sha1(b'blob '+str(len(raw)).encode()+b'\0'+raw).hexdigest()
        if actual!=blob:raise ValueError('archived input differs: '+name.decode())
    profile=snapshot/'SOURCE-PROFILE.json' 
    if profile.exists():raise ValueError('reserved generated source profile already tracked')
    profile.write_text(json.dumps({'schema':'dnbox-exact-git-render-profile/1','sourceCommit':sha,'sourceTree':tree,'method':'git archive exact HEAD; no working-tree source substitution'},sort_keys=True,indent=2)+'\n')
    ph=hashlib.sha256(profile.read_bytes()).hexdigest()
    slice_dir=snapshot/'tests/ui_defects/first_slice';sys.path.insert(0,str(slice_dir))
    spec=importlib.util.spec_from_file_location('dnbox_first_slice_run',slice_dir/'run.py');runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
    render=out/'render';render.mkdir();first=runner.run_one('current',snapshot,render)
    evaluation=out/'evaluation.json'
    evaluated=False
    if first.get('run',{}).get('exit')==0:
        process=subprocess.run([sys.executable,str(slice_dir/'evaluate.py'),'--render-dir',str(render/'current'),'--output',str(evaluation)],check=False)
        evaluated=process.returncode==0 and evaluation.exists()
    report=json.loads(evaluation.read_text()) if evaluation.exists() else first
    reasons=unmet(report,ph)
    if not evaluated:reasons.append('evaluation failed or unavailable')
    receipt={'sourceCommit':sha,'sourceTree':tree,'profileSha256':ph,'report':str(evaluation),'status':'FAIL' if reasons else 'PASS','unmet':reasons}
    (out/'qualification.json').write_text(json.dumps(receipt,sort_keys=True,indent=2)+'\n');print(json.dumps(receipt))
    return 1 if reasons else 0

if __name__=='__main__':raise SystemExit(main())
