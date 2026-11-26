"""Shape-only candidate on exact current sampled receiver, no legacy media export.
Falsification test for a fixed 7-partial per-zone model learned only from
late disjoint true source body, not any preset/control cue."""
from pathlib import Path
import math
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('base,trace,commit=render(False)')[0]
s=s.replace('def render(use_body_start):','def render(kind, schedule, end):')
s=s.replace('        start=lo if use_body_start and key==54 else 0','''        start=0
        if kind!='original':
            angular=math.pi*440.*2**((key-69)/12)/rate
            phase=np.arange(len(media),dtype='f8')*angular
            if kind=='basis7':
                # Fixed positions in late SFZ body; never use the actual preset
                # played frames, onset, note controls, holds or released audio.
                tr_lo=lo+(hi-lo)*3//5
                tr_hi=lo+(hi-lo)*4//5
                h=np.arange(1,8)
                phases=phase[tr_lo:tr_hi,None]*h
                basis=np.column_stack([np.ones(len(phases)),np.cos(phases),np.sin(phases)])
                y=media[tr_lo:tr_hi].astype('f8')/32768.
                fitted=np.linalg.lstsq(basis,y,rcond=None)[0].astype('f2').astype('f8')
                media_f=np.full(len(phase),fitted[0])
                for n in h:
                    media_f+=fitted[n]*np.cos(n*phase)+fitted[7+n]*np.sin(n*phase)
            else:
                zone=min(11,max(0,(key-34)//6+1)) if key>=34 else 0
                a=coeff[zone]
                media_f=sum((a[i]/128.)*np.sin((2*i+1)*phase) for i in range(4))
        else:
            media_f=media.astype('f4')/32768.''')
s=s.replace("                    media.astype('f4')/32768.)","                    media_f)")
s=s.replace('for on,k in events.get(now,[]):','for on,k in schedule.get(now,[]):').replace('if now>=57344:break','if now>=end:break')
exec(compile(s,'actual retained source clock and sampler boundary','exec'))
core=Path('sj6-hp-exact-exp-fit/kept-elite-1/firmware/src/dsp/dnbx_core.c')
assert pin(core)=='d58f87a1f598fb894a184ccd57f543a1ec451ee13aea020b6eec9e92ca0da625'
table=core.read_text().split('sj6_sub_harmonic[12][4]={')[1].split('};')[0]
coeff=[list(map(int,re.findall(r'[0-9]+',r))) for r in re.findall(r'\{([^}]+)\}',table)]
assert len(coeff)==12 and all(len(r)==4 for r in coeff)
def score(a,b,i,j):
 x,y=a[:,i:j].astype('f8'),b[:,i:j].astype('f8')
 n,d=np.linalg.norm(y),np.linalg.norm(x-y)
 assert n>0
 alt=np.linalg.norm(x)
 return {'level_delta_db':float(20*np.log10(alt/n)) if alt else None,
         'signed_dbr':float(20*np.log10(d/n)) if d else None,
         'reference_rms':float(n/np.sqrt(y.size))}
def both(ev,end,windows,test_holdout=False):
 out={}
 for v in ('original','bare','basis7'):
    a,_,receipt=render(v,ev,end)
    out[v]=a
    if not test_holdout:assert len(receipt)==1 and receipt[0][:2]==[49875,55] and receipt[0][3]==0.
 assert all(x.shape==out['original'].shape and np.isfinite(x).all() for x in out.values())
 return {k:{v:score(out[v],out['original'],i,j) for v in ('bare','basis7')} for k,i,j in windows}
scope0=[('onset_36',16384,18432),('held_36',24576,37634),('release_36',38434,47000),
        ('pending_54',49459,49875),('onset_54',49875,51923),('held_54',53971,81434),('release_54',82534,100000)]
scope1=[('onset_73',16384,18432),('held_73',24576,37634),('release_73',38434,47000)]
ret={'scope':'no asset export, frozen 7-harmonic per-zone synthesis from disjoint original SFZ loop-body windows at true source timing (descriptors in memory only); current real compiled original receiver, projected common control/timing with all cold note events retained, no terminal phase/time/gain fit',
     'core_sha':pin(BIN),'auth_older_container_sha':hashlib.sha256(data).hexdigest(),
     'cube_full_source':both(events,104000,scope0),
     'blind_key73_source':both({16384:[(True,(73,86))],38434:[(False,(73,0))]},48000,scope1,True)}
print(json.dumps(ret,indent=2,allow_nan=False))
