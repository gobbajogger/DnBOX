from pathlib import Path
import ctypes as c, hashlib, io, json, math, re, sys, tarfile, zipfile,struct
import numpy as np
o,b=map(Path,sys.argv[1:])
# Reuse authorized matching sample capsule decoder, no compiled app import.
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('ipa=')[1].split('nat=Path(')[0]
exec(compile('ipa='+s,'existing private SFZ container decoder','exec'))
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(data)=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
assert sha(sfz)=='3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
arc=b/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(arc.read_bytes())=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(arc) as t:
 def take(a):
  p=f'./program-2-{a}/'
  m=json.load(t.extractfile(p+'capture.json'))
  r=np.load(io.BytesIO(t.extractfile(p+'output.npy').read()),allow_pickle=False).copy()
  assert sha(r.tobytes())==m['pcm_sha256'] and m['active']==m['after'] and m['frames']==r.shape[1]==148684
  return r,m
 refs=dict((a,take(a)) for a in ('authored','dry','authored-repeat'))
assert np.array_equal(refs['authored'][0],refs['authored-repeat'][0])
unique={}
for g in regions:
    name=g['sample'].replace('\\','/').split('/')[-1]
    sr,media=pack[name];unique[name]=(sr,len(media))
print(json.dumps(dict(
    metadata_scope='aggregate only; no SFZ entries, PCM, paths, exact individually reconstructible table or original assets exported',
    distinct_zones=len(regions),distinct_media=len(unique),
    source_rates_hz=sorted(set(v[0] for v in unique.values())),
    decoded_PCM16_mono_total=sum(2*v[1] for v in unique.values()),
    decoded_F32_mono_total=sum(4*v[1] for v in unique.values()),
    region_frames_min_max=[min(v[1] for v in unique.values()),max(v[1] for v in unique.values())],
    loop_span_frames_min_max=[min(int(g['loop_end'])-int(g['loop_start']) for g in regions),max(int(g['loop_end'])-int(g['loop_start']) for g in regions)],
    hypothetical_metadata_only_bytes=dict(bytes_per_descriptor=24,total=24*len(regions),
        fields=['storage locator u32','stored frame count u32','end frame u32','loop start u32','loop stop u32','rate encoding+root+region bounds+flags u32; only if rates are represented by verified independent rate index']),
    hot_path='two mono sample words, float-to-double conversion, exact weighted lerp plus gain,narrow once; double cursor FMA; strict gated wrap only. Timing and cache architecture unmeasured',
    media_policy='Provider API fails closed if missing; storage source/authorization not established for adoption'),indent=2))
