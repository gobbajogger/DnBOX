from pathlib import Path
import hashlib,json,sys,struct
D=Path(__file__).resolve().parent;R=D.parent.parent
sys.path.insert(0,str(R/'restored-harness-inspect/helpers'))
import guarded_replay as g
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def one(model):
 ln=(D/f'{model}.log').read_text().splitlines()
 d=Path(next(s for s in ln if s.startswith(str(D/'dnbox-loaded-replay-'))))
 assert ln[-1]=='verified: byte-identical disposable virtual NOR'
 assert 'flash: 0 erases / programs' in (d/'run.log').read_text()
 elf=D.parent/'link-receipts'/f'{model}-on.never-install.elf'
 e,s,sec=g.symbols_and_sections(elf)
 rom=(d/'disposable.bin').read_bytes()
 rr=(d/'ram.bin').read_bytes()
 words=struct.unpack_from('<4I',rr,s['d9_test_timeline']-g.ELF_RAM)
 source={}
 for i in (0,1,2,3,5,6):
  q=0x9d000+8192*i; sz=g.u32(rom,q+12)
  source[str(i)]=g.sha(rom[q:q+32]+rom[q+256:q+256+sz])
 owners=json.loads((d/'owner.json').read_text())['owners']
 healthy=[o for o in owners if o['bank']==2 and o['status']==1 and o['bankB']==0 and o['statusB']==1 and o['arp']==1 and 0<=o['cutoff']<=127]
 return dict(model=model,elf=g.sha(e),nor=g.sha(rom),tape=sha(d/'scene.keys'),project_input_records=source,
     legacy_hit_timeline_words=words,prior_to_tape_owner=sum(o['ms']<2220 for o in healthy),
     first_valid_owner_ms=min((o['ms'] for o in healthy),default=None),
     raw_result=json.loads((d/'result.json').read_text()),owner=owners,
     pcm=sha(d/'audio.s16le'),ram=g.sha(rr), unchanged_disposable_virtual_nor=True)
a,b=[one(m) for m in ('elite','pro')]
assert a['tape']==b['tape']=='d08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271'
assert a['project_input_records']==b['project_input_records']
print(json.dumps(dict(unchanged_tape_events=922, paired_project_records_exact=True,
    result=[a,b],scope='strict runner diagnostic, not release: no producer-frame admission; embedded existing diagnostic triggers measured separately.'),indent=2))
