"""Classify present receipts by acquisition boundary, not label alone."""
from pathlib import Path
import json,tarfile,hashlib
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def scan(p,tag):
 p=Path(p)
 with tarfile.open(p) as t:
  ret=[]
  for n in t.getnames():
   if not n.endswith('/capture.json') or '._' in n:continue
   d=json.load(t.extractfile(n))
   if tag=='raw' and 'program-2-' not in n:continue
   v={'receipt':n,'component':d.get('component',d.get('au_component')),
    'rate':d['rate'],'frames':d['frames'],'described_source':d.get('source_stage'),
    'explicit_acquisition':d.get('stage'),'profile':d.get('profile'),
    'cue':d.get('cue_sha256'),'sha':d.get('pcm_sha256',d.get('input_pcm_sha256'))}
   assert v['rate']==44100 and v['frames']==148684
   if tag=='raw':
    assert d['active']==d['after'] and d['cue_sha256']=='17b7b5389af054fd713c31acadbf170955c4146628872c9482ed23ce07f33943'
    assert d['component']=='aumu/j6ct/AuPr'
    assert set(k.rsplit('/',1)[1] for k in t.getnames() if k.startswith(n.rsplit('/',1)[0]+'/') and '/._' not in k and not k.endswith('/'))<={'capture.json','output.npy','controls.json'}
    v['source_class']='native installed terminal stereo only'
   else:
    assert v['component'] in ('aufx/hpas/appl','aufx/dist/appl')
    assert 'current J6 internal PCM not observed' in v['explicit_acquisition'] or 'current J6 node not proven' in v['explicit_acquisition']
    assert (v['profile'].startswith('external Brainwave') or v['profile'].startswith('external source-bound'))
    v['source_class']='independent Apple leaf on external renderer tape, not private AU node'
   ret.append(v)
  return dict(file_sha=sha(p),records=ret)
orig=scan('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz','raw')
hp=scan('sj6-hp-route-native/SJ6-Portable-HP-Zero-Crusher-References.tgz','hpf')
crush=scan('recovered-sound/SJ6-Brainwave-Warm-Main-Sub-Crusher-native.private.tgz','crusher')
assert len(orig['records'])==3 and len(hp['records'])==6 and len(crush['records'])==2
assert orig['file_sha']=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
assert hp['file_sha']=='444407d73a7dad329d7b3818ebc2445664b98d2a294cb97db40ab85d80bc895a'
pair=json.loads(Path('sj6-postvoice-split/audit.json').read_text())
print(json.dumps(dict(corpus=[orig,hp,crush],
 first_shared_missing='signed installed oscSection AVAudioMixerNode stereo out ahead of Unison and instrument HP, Raw current cue, no stage inferred',
 strongest_matched_relative_control=dict(
   description='True current in-process J60 enabled vs bypass genuine endpoint, conditional rebuilt source path and authored full/cold dry. Not a shared installed pre-J60 node capture.',
   expected_gap_db=pair['full']['relative_wet_amplitude_gap_after_dividing_by_each_arms_dry_amplitude_db'][3]),
 no_other_new_audio_request=True),indent=2))
