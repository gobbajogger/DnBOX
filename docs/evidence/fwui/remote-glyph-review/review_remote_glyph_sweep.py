import argparse,array,gzip,hashlib,json,pathlib,sys
parser=argparse.ArgumentParser();parser.add_argument('--evidence',type=pathlib.Path,required=True);parser.add_argument('--output',type=pathlib.Path,required=True);args=parser.parse_args()
r=args.evidence;n=0;constant=[];contexts=set()
for p in sorted(r.glob('t*-v*-p*.json')):
 d=json.loads(p.read_text());ctx=d['liveContext'];assert ctx=={'live':0,'age':4294967295,'envelope':0}
 deps=d['dependencyVector'];assert len(deps)==(97 if d['target']==0 else 100)
 keys={(x['target'],x['voice'],x['slot']) for x in deps};assert len(keys)==len(deps)
 key=(d['target'],d['voice'],d['slot']);assert key not in contexts;contexts.add(key)
 assert next(x['raw'] for x in deps if (x['target'],x['voice'],x['slot'])==key)==d['storedRaw']
 z=p.with_suffix('.rgb565be.gz').read_bytes();b=gzip.decompress(z)
 assert hashlib.sha256(z).hexdigest()==d['gzipSha256'];assert hashlib.sha256(b).hexdigest()==d['rawSha256']
 assert len(z)==d['gzipBytes'] and len(b)==d['rawBytes'];assert len(b)==6144*(d['maximumRaw']+1)
 previous=None;hs=[];changes=[];bounds=[]
 for raw in range(d['maximumRaw']+1):
  frame=b[6144*raw:6144*(raw+1)];hs.append(hashlib.sha256(frame).hexdigest());pix=array.array('H',frame)
  if sys.byteorder=='little':pix.byteswap()
  ink=[i for i,x in enumerate(pix) if x!=d['backgroundRgb565']];assert ink
  box=[min(i%64 for i in ink),min(i//64 for i in ink),max(i%64 for i in ink)+1,max(i//64 for i in ink)+1]
  # Producer bounds use exclusive right/bottom, checked against actual bytes.
  bounds.append(box);assert box[0]>0 and box[1]>0 and box[2]<64 and box[3]<48
  if previous is not None:changes.append(sum(a!=b for a,b in zip(previous,pix)))
  previous=pix
 assert hs==d['frameSha256'];assert changes==d['adjacentChangedPixels'];assert bounds==d['inkBounds']
 assert len(set(hs))==d['distinctFrames'];assert [v['raw'] for v in d['values']]==list(range(d['maximumRaw']+1))
 if len(set(hs))==1:constant.append(key)
 n+=len(hs)
assert len(contexts)==63 and n==7434 and constant==[(0,0,5)]
report={'status':'PASS','controls':len(contexts),'frames':n,'constantControls':constant,'verified':'gzip/raw/per-frame hashes, context keys, idle state, dependencies, raw coverage, ink bounds, adjacent differences','limits':'Independent evidence recomputation and source review, not native replay, live temporal behavior, whole-layout geometry or browser acceptance.'}
args.output.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
