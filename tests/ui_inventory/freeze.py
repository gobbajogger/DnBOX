#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Export/authenticate one local Git tree; no network or Git mutation."""
import argparse,hashlib,json,os,re,subprocess,tarfile,tempfile
from pathlib import Path

def freeze(repo,tree,out):
    repo=repo.resolve();out=out.resolve()
    if not re.fullmatch(r'[0-9a-f]{40}',tree):raise ValueError('exact tree/commit SHA required')
    if out.exists():raise ValueError('new output directory required')
    resolved=subprocess.check_output(['git','rev-parse',tree+'^{tree}'],cwd=repo,text=True).strip()
    raw=subprocess.check_output(['git','ls-tree','-rz','--full-tree',resolved],cwd=repo)
    out.mkdir(parents=True)
    with tempfile.TemporaryFile() as archive:
        subprocess.run(['git','archive',resolved],cwd=repo,stdout=archive,check=True);archive.seek(0)
        with tarfile.open(fileobj=archive,mode='r:') as tar:tar.extractall(out,filter='data')
    rows=[]
    for item in raw.split(b'\0'):
        if not item:continue
        metadata,name=item.split(b'\t',1);mode,kind,digest=metadata.decode().split();name=name.decode()
        if kind!='blob':raise ValueError('unrestored nonblob tree entry '+name)
        path=out/name;data=os.readlink(path).encode() if path.is_symlink() else path.read_bytes()
        if hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()!=digest:raise ValueError('exported Git blob mismatch '+name)
        rows.append({'path':name,'sha1':digest})
    reference={'tree':resolved,'files':rows}
    (out/'production-tree.json').write_text(json.dumps(reference,indent=2)+'\n')
    print(json.dumps({'status':'PASS','tree':resolved,'files':len(rows),'output':str(out),'scope':'Exact local Git-tree export only; no runtime qualification.'}))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--repo',type=Path,required=True);p.add_argument('--tree',required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    try:freeze(a.repo,a.tree,a.output)
    except (OSError,ValueError,subprocess.SubprocessError) as e:print(json.dumps({'status':'FAIL','reason':str(e)}));raise SystemExit(1)
