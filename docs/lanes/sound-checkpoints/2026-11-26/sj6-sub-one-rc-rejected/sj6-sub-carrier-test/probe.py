"""Shape-only candidate on exact current sampled receiver, no legacy media export.
Compare a bare currently used synthetic carrier and fixed 1 ms RC on same
generated input, same installed reference control/marker trajectories."""
from pathlib import Path
from scipy.signal import lfilter
import math
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('base,trace,commit=render(False)')[0]
s=s.replace('def render(use_body_start):','def render(kind, schedule, end):')
s=s.replace('        start=lo if use_body_start and key==54 else 0','''        start=0
        if kind!='original':
            zone=min(11,max(0,(key-34)//6+1)) if key>=34 else 0
            a=coeff[zone]
            phase=np.arange(len(media),dtype='f8')*(math.pi*440.*2**((key-69)/12)/rate)
            # Use only existing independent compact generator coefficients.
            media_f=sum((a[i]/128.)*np.sin((2*i+1)*phase) for i in range(4))
            if kind=='fixed_rc':
                z=math.exp(-1./(float(rate)*.001))
                media_f=lfilter([1-z],[1,-z],media_f)
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
 for v in ('original','bare','fixed_rc'):
    a,_,receipt=render(v,ev,end)
    out[v]=a
    if not test_holdout:assert len(receipt)==1 and receipt[0][:2]==[49875,55] and receipt[0][3]==0.
 assert all(x.shape==out['original'].shape and np.isfinite(x).all() for x in out.values())
 return {k:{v:score(out[v],out['original'],i,j) for v in ('bare','fixed_rc')} for k,i,j in windows}
scope0=[('onset_36',16384,18432),('held_36',24576,37634),('release_36',38434,47000),
        ('pending_54',49459,49875),('onset_54',49875,51923),('held_54',53971,81434),('release_54',82534,100000)]
scope1=[('onset_72',16384,18432),('held_72',24576,37634),('release_72',38434,47000)]
ret={'scope':'no asset export, fixed global one-ms RC on existing generated Sub waveform; current real compiled original receiver, projected common control/timing with all cold note events retained, no terminal phase/time/gain fit',
     'core_sha':pin(BIN),'auth_older_container_sha':hashlib.sha256(data).hexdigest(),
     'cube_full_source':both(events,104000,scope0),
     'blind_key72_source':both({16384:[(True,(72,86))],38434:[(False,(72,0))]},48000,scope1,True)}
print(json.dumps(ret,indent=2,allow_nan=False))
