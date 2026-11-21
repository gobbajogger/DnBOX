#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Directly extractable lossless PNG family tar.xz parts, bounded for transfer."""
import argparse,hashlib,io,json,lzma,tarfile
from pathlib import Path
from PIL import Image
LIMIT=800000

def compressed(rows):
    target=io.BytesIO();seen={}
    with tarfile.open(fileobj=target,mode='w') as tar:
        for name,data,pixels in rows:
            item=tarfile.TarInfo(name);item.mtime=0;item.mode=0o644
            if pixels and pixels in seen:
                item.type=tarfile.LNKTYPE;item.linkname=seen[pixels];tar.addfile(item)
            else:
                item.size=len(data);tar.addfile(item,io.BytesIO(data))
                if pixels:seen[pixels]=name
    return lzma.compress(target.getvalue(),preset=6)

def pack(source,out):
    out.mkdir(parents=True,exist_ok=False)
    report=json.loads((source/'report.json').read_text());groups={};mapping=[];receipts=[]
    methods={}
    if (source/'frame-methods.json').is_file():methods={r['path']:r['evidenceKind'] for r in json.loads((source/'frame-methods.json').read_text())}
    for name,frames in sorted(report['frames'].items()):
        for instance,frame in enumerate(frames):
            path=source/frame['path']
            if hashlib.sha256(path.read_bytes()).hexdigest()!=frame['sha256']:raise ValueError('changed PNG')
            with Image.open(path) as im:
                im.load();im=im.convert('RGB');rgb=im.tobytes();encoded=io.BytesIO();im.save(encoded,format='PNG',compress_level=0)
            data=encoded.getvalue()
            with Image.open(io.BytesIO(data)) as im:
                im.load()
                if im.size!=(240,240) or im.convert('RGB').tobytes()!=rgb:raise ValueError('changed pixels')
            stable_path=f'renders/{name}/capture-{instance:02d}.png';pixel_hash=hashlib.sha256(rgb).hexdigest()
            groups.setdefault(name.split('-')[0],[]).append((stable_path,data,pixel_hash))
            mapped=dict(frame,storedPath=stable_path,storedPNGsha256=hashlib.sha256(data).hexdigest(),pixelSha256=pixel_hash)
            if 'evidenceKind' not in mapped:mapped['evidenceKind']=methods.get(frame['path'],'NOT_CLASSIFIED')
            mapping.append(mapped)
    def save(family,rows,label='0'):
        data=compressed(rows)
        if len(data)>LIMIT:
            if len(rows)<2:raise ValueError('oversized archive entry')
            midpoint=len(rows)//2;save(family,rows[:midpoint],label+'a');save(family,rows[midpoint:],label+'b');return
        name=f'{family}-{label}.tar.xz';(out/name).write_bytes(data)
        # Decode and authenticate the actual delivered archive, including links.
        with tarfile.open(fileobj=io.BytesIO(data),mode='r:xz') as tar:
            for path,expected,_ in rows:
                actual=tar.extractfile(path).read()
                if actual!=expected:raise ValueError('archive roundtrip mismatch')
        receipts.append(dict(path=name,bytes=len(data),sha256=hashlib.sha256(data).hexdigest(),entries=[r[0] for r in rows]))
    for family,rows in sorted(groups.items()):save(family,rows)
    metadata=[('frame-map.json',(json.dumps(mapping,indent=2)+'\n').encode(),None)]
    for name in ('report.json','source-inventory.json','source-closure.json','render.log','production-tree.json','production-tree-receipt.json','runner-inputs.json','native-display-slices.json','stable-renderer-ids.tsv','verification.json','frame-methods.json','delivery-verifier.py'):
        p=source/name
        if p.is_file():metadata.append((name,p.read_bytes(),None))
    for path in sorted((source/'runner-inputs').glob('*')):
        if path.is_file():metadata.append(('runner-inputs/'+path.name,path.read_bytes(),None))
    metadata.append(('delivery-packer.py',Path(__file__).read_bytes(),None))
    save('metadata',metadata)
    index=dict(schema=1,status=report['status'],F1=report['F1'],sourceClosure=report['sourceClosure'],
               frames=len(mapping),stableStateIDs=len(report['frames']),totalBytes=sum(r['bytes'] for r in receipts),
               maxPartBytes=max(r['bytes'] for r in receipts),archives=receipts,packerSHA256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
               note='Direct tar.xz extraction yields stable state/instance PNG paths. PNG compression=0 exposes shared pixels to xz; exact decoded RGB verified. Byte-identical PNGs use archive-local hardlinks. No visual approval.')
    (out/'index.json').write_text(json.dumps(index,indent=2)+'\n');print(json.dumps({k:v for k,v in index.items() if k!='archives'}))
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();pack(a.source,a.output)
