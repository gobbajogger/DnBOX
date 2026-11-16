# SPDX-License-Identifier: GPL-3.0-only
"""Isolated source-pinned word/error/guard tests. No app, firmware or device execution."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
def digest(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
 a=argparse.ArgumentParser();a.add_argument('--source-pin',required=True);a.add_argument('--out',type=Path,required=True);a.add_argument('--compact',type=Path);a.add_argument('--raw',type=Path);a=a.parse_args()
 root=Path(__file__).resolve().parent.parent;mf=root/'SOURCE-MANIFEST.json'
 if digest(mf)!=a.source_pin: raise SystemExit('source manifest pin mismatch')
 manifest=json.loads(mf.read_bytes());names=set()
 for e in manifest['files']:
  name=e['path'];rel=Path(name)
  if rel.is_absolute() or '..' in rel.parts or name in names:raise SystemExit('invalid source path')
  names.add(name);p=root/rel
  if p.stat().st_size!=e['bytes']or digest(p)!=e['sha256']:raise SystemExit('changed source: '+name)
 actual={str(p.relative_to(root))for p in root.rglob('*')if p.is_file()}
 if actual!=names|{'SOURCE-MANIFEST.json'}:raise SystemExit('source closure mismatch')
 if bool(a.compact)!=bool(a.raw):raise SystemExit('both fixture inputs are required')
 if a.compact:
  if a.compact.stat().st_size!=165292 or digest(a.compact)!='a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5':raise SystemExit('compact fixture pin mismatch')
  if a.raw.stat().st_size!=852176 or digest(a.raw)!='0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5':raise SystemExit('raw fixture pin mismatch')
 cc=shutil.which('cc')
 if not cc:raise SystemExit('C compiler unavailable')
 out=a.out.resolve();out.mkdir(parents=True,exist_ok=False);runs=[]
 def execute(name,cmd,env=None):
  q=subprocess.run(list(map(str,cmd)),capture_output=True,env=env);(out/(name+'.stdout')).write_bytes(q.stdout);(out/(name+'.stderr')).write_bytes(q.stderr);runs.append(dict(name=name,command=list(map(str,cmd)),exit=q.returncode,utc=datetime.datetime.now(datetime.timezone.utc).isoformat()));(out/'COMMANDS.private.json').write_text(json.dumps(runs,indent=2)+'\n');return q
 for variant,flags in [('o2',['-O2']),('os',['-Os']),('sanitize',['-O1','-g','-fsanitize=address,undefined,float-cast-overflow','-fno-sanitize-recover=all','-fno-omit-frame-pointer'])]:
  tests=['extract_test']+(['test_words','public_guard_test']if a.compact else [])
  for test in tests:
   sources=[root/'compact-reader'/(test+'.c')]+([]if test=='extract_test'else[root/'compact-reader/compact.c'])
   binary=out/(variant+'-'+test);cmd=[cc,'-std=gnu99',*flags,'-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-builtin','-ffp-contract=off','-fno-fast-math','-I'+str(root/'compact-reader'),*sources,'-o',binary];q=execute(variant+'-'+test+'-build',cmd)
   if q.returncode:raise SystemExit(q.stderr.decode())
   args=[binary]+([]if test=='extract_test'else[a.compact.resolve(),a.raw.resolve()]);env=os.environ.copy();env.pop('ASAN_OPTIONS',None);env['UBSAN_OPTIONS']='halt_on_error=1:print_stacktrace=1';q=execute(variant+'-'+test+'-default',args,env)
   if variant=='sanitize':
    if q.returncode and not(b'LeakSanitizer'in q.stderr and b'ptrace'in q.stderr):raise SystemExit(q.stderr.decode())
    env['ASAN_OPTIONS']='detect_leaks=0';q=execute(variant+'-'+test+'-leaks0',args,env)
   if q.returncode or q.stderr:raise SystemExit(q.stderr.decode())
   print(variant,test,q.stdout.decode().strip(),flush=True)
 result={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'source_manifest_sha256':a.source_pin,'compiler_path':cc,'compiler_sha256':digest(cc),'scope':'Own host words/errors/read-only guard pages only; no original app/native CPU/full engine/fit/audio/device acceptance','lsan':'Default sanitizer and same-binary leak-disabled runs recorded separately; disabled leaks are NOT leak proof','runs':runs}
 (out/'RESULT.private.json').write_text(json.dumps(result,indent=2)+'\n')
if __name__=='__main__':main()
