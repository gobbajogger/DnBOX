"""Reproduce the three compact diagnostic arms from exact installed AU readbacks.
Only control/cue receipts enter render; no reference PCM is read by this script."""
from pathlib import Path
import tarfile,zipfile,json, collections,hashlib,sys,subprocess
from struct import pack
au=Path(sys.argv[1]); bank=Path(sys.argv[2]); output=Path(sys.argv[3]); binary=Path(sys.argv[4]).resolve()
def digest(f):return hashlib.sha256(Path(f).read_bytes()).hexdigest()
assert digest(au)=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
assert digest(bank)=='ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8'
ids=['1E5E5DDF-B5C6-4503-BDBC-CAE9CD1D0A94','6A0861A3-6D2F-4A08-922A-7B616E3F6B3E','9F6F6313-DD03-437F-B2EC-BCF7904DF871']
cue=[(16384,48,100),(38434,48,0),(49459,60,96),(49459,64,80),(49459,67,88),(82534,60,0),(82534,64,0),(82534,67,0)]
fx={11,16,17,22,23,24,40,41,107}
mapped={57,93,95,97,98,99,101,102,103,104,108,131,138,139,140,141,184,185,186,187,188,189,190,191,192,193,194,199,200,0,1,3,7,8,11,12,13,14,15,16,17,22,23,24,25,26,27,28,32,33,35,36,37,38,39,40,41,42,43,44,59,60,61,62,72,73,74,75,78,79,82,83,86,87,88,89,91,94,96,100,105,106,107,115,116,118,119,120,121,122,123,132,133,134,135,136,137,180,181,195,196,197,198,201,202,205,206,208,209,210,211,215,216,217}
records={}
with zipfile.ZipFile(bank) as z:
 for name in z.namelist():
  if not name.startswith('J6presets/') or not name.endswith('.superj6.bank.json'):continue
  for row in json.loads(z.read(name))['presets']:
   if row['uid'] in ids:
    assert row['uid'] not in records
    records[row['uid']]=row
assert set(records)==set(ids)
output.mkdir(exist_ok=False)
results=[]
def get(t,p):return json.load(t.extractfile(p))
with tarfile.open(au) as t:
 for i,uid in enumerate(ids):
  sets={}
  for arm in ('authored','dry','authored-repeat'):
   cap=get(t,f'./program-{i}-{arm}/capture.json')
   rec=records[uid]; wanted=cap['wanted']
   assert wanted['program_uid']==uid and wanted['program']==rec['name']
   fixture_sha=hashlib.sha256(json.dumps(rec,sort_keys=True,ensure_ascii=False,separators=(',',':'),allow_nan=False).encode()).hexdigest()
   # Native requested record hash comes from the release audit, not always
   # byte-identical to separately exported bank serialization. Record both.
   assert cap['component']=='aumu/j6ct/AuPr' and cap['component_version']==67072
   assert cap['cue_sha256']=='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
   assert cap['rate']==44100 and cap['warm_frames']==16384 and cap['frames']==148684
   assert [(e['frame'],e['note'],e['velocity']) for e in cap['schedule']]==cue
   active=cap['active']; wr={int(x['address']):x for x in active}; assert len(active)==len(wr)==170
   assert cap['active']==cap['after']
   sets[arm]=wr
   assert sum(cap['blocks'])==148684 and min(cap['blocks'])>0 and max(cap['blocks'])==128
   assert all(wr[k]['value']==0 for k in (63,142,212))
   deltas={int(k) for k in wr if pack('<f',wr[k]['value'])!=pack('<f',sets['authored'][k]['value'])}
   assert not (deltas-fx) if arm=='dry' else not deltas
   if arm=='authored-repeat':continue
   events=collections.defaultdict(list)
   for at,key,vel in cue:events[at].append((key,vel))
   lines=[str(rec['seqTempo']),str(len(wr))]
   lines+=['%d %.9g'%(k,w['value']) for k,w in sorted(wr.items())]
   lines.append(str(len(cap['blocks'])))
   at=0
   for n in cap['blocks']:
    ev=events.pop(at,[])
    lines.append(' '.join(str(k) for k in [n,len(ev),*(q for ks in ev for q in ks)]))
    at+=n
   assert not events
   name=f'{i}-{arm}'
   cfg=output/f'{name}.private.txt'; out=output/f'{name}.private.f32'
   cfg.write_text('\n'.join(lines)+'\n')
   subprocess.run([str(binary),str(cfg),str(out)],check=True)
   assert out.stat().st_size==148684*8
   results.append(dict(uid=uid,name=rec['name'],arm=arm,scope='typed compact projection only',
        pin_sha=wanted['record_sha256'],separate_export_record_sha=fixture_sha,bpm_from_auth_export=rec['seqTempo'],
        receipt_sha=digest(cfg),compact_sha=digest(out),reference_pcm_sha=cap['pcm_sha256'],
        after_verified=True,cue_sha=cap['cue_sha256'],changed_live_ids=sorted(deltas),authored_overlap=cap['authored_overlap'],
        original_readback_differences=cap['authored_readback_mismatch'],
        unsupported_nonzero={k:w['identifier'] for k,w in wr.items() if k not in mapped and w['value']!=0.0}))
json.dump(results,open(output/'manifests.json','w'),indent=2)
