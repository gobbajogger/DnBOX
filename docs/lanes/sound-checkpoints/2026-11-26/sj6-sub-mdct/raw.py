from pathlib import Path
import json,sys,struct,numpy as np
s=Path(__file__).with_name('measure.py')
exec(compile(s.read_text().split('row={}')[0],str(s),'exec'))
p=Path('sj6-sub-stationary-cycle/measure.py')
frag=p.read_text().split('arc=')[1].split('def make(')[0]
exec(compile('arc='+frag,str(p),'exec'))
w=json.loads(Path(__file__).with_name('frontier.json').read_text())[-1]
assert w['payload_bytes']==112991
other={}
for g in regions:
    name=g['sample'].replace('\\','/').split('/')[-1]
    sr,a=pack[name];root=g['pitch_keycenter']
    N,k=map(int,w['options'][root].split(':'))
    c,win=forward(a.astype('f8')/32768.,N)
    ii=np.argsort(-abs(c),axis=1,kind='stable')[:,:k]
    a2=np.zeros_like(c);np.put_along_axis(a2,ii,np.rint(np.take_along_axis(c,ii,axis=1)*512.)/512.,axis=1)
    # Existing original sampler reads PCM16 through exact no-copy compatible
    # route. Deliberately retain raw clock, controls and gate; only tape changes.
    y=np.round(backward(a2,win,len(a))*32768.).clip(-32768,32767).astype('i2')
    other[name]=(sr,y)
new,p2=run(other);assert p2==pin
win={'onset':(16384,18589),'first_hold':(20794,38434),'first_release':(38434,39094),'chord':(53869,82534),'final_release':(82534,83194),'full':(16384,148684)}
print(json.dumps(dict(current_pin=pin,shared_before_BREAK_pool_limit=112*1024,operating_point=w,
 actual_current_sub_replay={k:score(new[:,i:j],old[:,i:j]) for k,(i,j) in win.items()},
 original_replay_hash=sha(old.tobytes()),unchanged_voice_state_and_event_times=True),indent=2))
