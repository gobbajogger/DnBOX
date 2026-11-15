#!/usr/bin/env python3
"""Fixture-free source checks. Never loads or executes an original app."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,subprocess
P=Path(__file__).resolve().parents[1]
def require(ok,message):
 if not ok:raise ValueError(message)
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--out',type=Path,required=True);ap.add_argument('--manifest-sha256',required=True);a=ap.parse_args()
 raw=(P/'SOURCE-MANIFEST.json').read_bytes();require(hashlib.sha256(raw).hexdigest()==a.manifest_sha256,'source manifest pin')
 manifest=json.loads(raw);expected={e['path']for e in manifest['files']}|{'SOURCE-MANIFEST.json'}
 actual={str(f.relative_to(P))for f in P.rglob('*')if f.is_file()}
 require(actual==expected,'unlisted or missing package files')
 require(not any(f.is_symlink()for f in P.rglob('*')),'package symlinks are not admitted')
 for e in manifest['files']:
  path=(P/e['path']).resolve();require(P.resolve()in path.parents,'source path outside package');b=path.read_bytes()
  require(len(b)==e['bytes']and hashlib.sha256(b).hexdigest()==e['sha256'],'source identity: '+e['path'])
  require(hashlib.sha1(b'blob '+str(len(b)).encode()+b'\0'+b).hexdigest()==e['git_blob'],'source Git blob: '+e['path'])
 out=a.out.resolve();require(not out.is_relative_to(P.resolve()),'outputs must be outside source package');out.mkdir(parents=True,exist_ok=False);rows=[]
 for variant,flags in [('o2',['-O2']),('os',['-Os']),('sanitize',['-O1','-g','-fsanitize=address,undefined,float-cast-overflow','-fno-sanitize-recover=all','-fno-omit-frame-pointer'])]:
  common=P/'sampler/DunneCore/Common';numeric=out/(variant+'-numeric.o')
  # numeric_boundary.c includes fma32.c: do NOT compile/link a second copy.
  cmd=['cc','-std=c11',*flags,'-ffp-contract=off','-fno-fast-math','-I'+str(common),'-c',str(common/'numeric_boundary.c'),'-o',str(numeric)]
  q=subprocess.run(cmd,capture_output=True);(out/(variant+'-numeric.stdout')).write_bytes(q.stdout);(out/(variant+'-numeric.stderr')).write_bytes(q.stderr);require(q.returncode==0,q.stderr.decode())
  for suite in ['lifecycle','source_laws','review_lifecycle','borrow_pcm16','saga','saga_regressions']:
   o=out/(variant+'-'+suite);o.mkdir();sampler=suite in ['lifecycle','source_laws','review_lifecycle','borrow_pcm16']
   sources=[P/'sampler/tests'/(suite+'.cpp'),*sorted((P/'sampler/DunneCore').rglob('*.cpp'))]if sampler else[P/'timing'/('tests.cpp'if suite=='saga'else'review_regressions.cpp')]
   inc=[P/'sampler/include',common,P/'sampler/DunneCore/Sampler']if sampler else[P/'timing']
   cmd=['c++','-std=c++17',*flags,'-ffp-contract=off','-fno-fast-math',*['-I'+str(i)for i in inc],*map(str,sources),*([str(numeric)]if sampler else[]),'-o',str(o/'tests')]
   q=subprocess.run(cmd,capture_output=True);(o/'build.stdout').write_bytes(q.stdout);(o/'build.stderr').write_bytes(q.stderr);require(q.returncode==0,q.stderr.decode())
   env=os.environ.copy();env.pop('ASAN_OPTIONS',None);env['UBSAN_OPTIONS']='halt_on_error=1:print_stacktrace=1'
   q=subprocess.run([str(o/'tests')],capture_output=True,env=env);default=q.returncode;(o/'default.stdout').write_bytes(q.stdout);(o/'default.stderr').write_bytes(q.stderr)
   if variant=='sanitize':
    if q.returncode:require(b'LeakSanitizer'in q.stderr and b'ptrace'in q.stderr,q.stderr.decode())
    env['ASAN_OPTIONS']='detect_leaks=0';q=subprocess.run([str(o/'tests')],capture_output=True,env=env);(o/'leaks0.stdout').write_bytes(q.stdout);(o/'leaks0.stderr').write_bytes(q.stderr)
   require(q.returncode==0 and not q.stderr,q.stderr.decode())
   row=dict(completed_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),variant=variant,suite=suite,default_exit=default,final_exit=q.returncode,stdout=q.stdout.decode(),stderr_bytes=len(q.stderr),source_manifest_sha256=a.manifest_sha256,scope='Fixture-free host source checks only; no original execution, full preset audio, native fit or device evidence');rows.append(row);(out/'RESULT.json').write_text(json.dumps(rows,indent=2)+'\n');print(json.dumps(row),flush=True)
 require(all(hashlib.sha256((P/e['path']).read_bytes()).hexdigest()==e['sha256']for e in manifest['files']),'source changed during checks')
if __name__=='__main__':main()
