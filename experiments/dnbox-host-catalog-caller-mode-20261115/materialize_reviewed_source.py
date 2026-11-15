#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Optional exact regular-source materializer; preserves authenticated executable modes.

Only creates a new output tree. Existing outputs are never changed. Source and
manifest must be immutable through the call. A failed copy is not a usable tree;
partial outputs are retained for diagnosis, never silently declared complete.
No archive extraction, symlinks, network, shell execution or security settings.
"""
import argparse,hashlib,json,os,stat
from pathlib import Path
def sha(b):return hashlib.sha256(b).hexdigest()
def materialize(source,manifest,pin,output):
 source=Path(source).resolve(strict=True);manifest=Path(manifest);output=Path(output)
 raw=manifest.read_bytes()
 if sha(raw)!=pin:raise ValueError('manifest pin mismatch')
 data=json.loads(raw);files=data['files'];seen=set();checked=[]
 if not isinstance(files,list)or not files:raise ValueError('empty or invalid files')
 if output.exists()or output.is_symlink():raise ValueError('output already exists')
 out=output.parent.resolve(strict=True)/output.name
 if out==source or source in out.parents:raise ValueError('output inside source')
 for f in files:
  name=f['path'];rel=Path(name)
  if not isinstance(name,str)or not name or rel.is_absolute()or '..'in rel.parts or name in seen or str(rel)!=name:raise ValueError('invalid/duplicate relative path')
  seen.add(name)
  if f['mode']not in('100644','100755'):raise ValueError('unsupported source mode')
  if type(f['bytes'])is not int or f['bytes']<0:raise ValueError('invalid byte length')
  p=source/rel
  for a in [p,*p.parents]:
   if a==source:break
   if a.is_symlink():raise ValueError('symlink source component')
  if not stat.S_ISREG(p.stat().st_mode):raise ValueError('not regular source')
  b=p.read_bytes()
  if len(b)!=f['bytes']or sha(b)!=f['sha256']:raise ValueError('source bytes mismatch: '+name)
  checked.append((p,f))
 output.mkdir(exist_ok=False)
 for p,f in checked:
  b=p.read_bytes()
  if len(b)!=f['bytes']or sha(b)!=f['sha256']:raise ValueError('source changed during copy')
  dst=output/f['path'];dst.parent.mkdir(parents=True,exist_ok=True);dst.write_bytes(b)
  dst.chmod(0o755 if f['mode']=='100755'else 0o644)
 for p,f in checked:
  dst=output/f['path'];b=dst.read_bytes();mode=dst.stat().st_mode&0o777
  if len(b)!=f['bytes']or sha(b)!=f['sha256']or mode!=(0o755 if f['mode']=='100755'else 0o644):raise ValueError('output verification failed')
 if manifest.read_bytes()!=raw:raise ValueError('manifest changed during copy')
 return {'schema':'dnbox-regular-source-materialization/1','sourceManifestSHA256':pin,'files':len(checked),'bytes':sum(f['bytes']for _,f in checked),'allDestinationSHA256AndModes':True,'output':str(output),'limits':'Regular immutable sources only; not a native build, runtime test or integration admission. Partial output on error must not be consumed.'}
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',required=True,type=Path);p.add_argument('--manifest',required=True,type=Path);p.add_argument('--pin',required=True);p.add_argument('--out',required=True,type=Path);a=p.parse_args();print(json.dumps(materialize(a.source,a.manifest,a.pin,a.out),indent=2))
if __name__=='__main__':main()
