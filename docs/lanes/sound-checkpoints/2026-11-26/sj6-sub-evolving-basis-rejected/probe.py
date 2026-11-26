"""Source-time Sub diagnostic on exact current sampled receiver, no media export.
Coefficients are kept in RAM only; not a firmware format or implementation."""
from pathlib import Path
import math
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('base,trace,commit=render(False)')[0]
s=s.replace('def render(use_body_start):','def render(kind, schedule, end):')
s=s.replace('        start=lo if use_body_start and key==54 else 0','''        start=0
        if kind!='original':
            angular=math.pi*440.*2**((key-69)/12)/rate
            phase=np.arange(len(media),dtype='f8')*angular
            if kind=='clockbasis':
                # Predeclared source clock anchors, never use output cues/cursor windows.
                # 100 ms calibration centered on 125+250i ms, other times blind.
                hop=rate//4
                half=rate//20
                h=np.arange(1,8)
                anchors=np.arange(hop//2,len(media)-half,hop)
                assert len(anchors)>=2
                fits=[]
                for j in anchors:
                    ix=slice(j-half,j+half)
                    phases=phase[ix,None]*h
                    basis=np.column_stack([np.ones(len(phases)),np.cos(phases),np.sin(phases)])
                    y=media[ix].astype('f8')/32768.
                    fits.append(np.linalg.lstsq(basis,y,rcond=None)[0])
                fits=np.array(fits)
                # Real phase and level drift come solely from the sampled source;
                # complex pairs interpolate along the shortest arc, envelope separately.
                ii=np.arange(len(media))
                media_f=np.interp(ii,anchors,fits[:,0].astype('f2').astype('f8'))
                for n in h:
                    harmonic=fits[:,n]-1j*fits[:,7+n]
                    rad=np.unwrap(np.angle(harmonic))
                    mag=abs(harmonic).astype('f2').astype('f8')
                    rad=rad.astype('f2').astype('f8')
                    media_f+=np.interp(ii,anchors,mag)*np.cos(n*phase+np.interp(ii,anchors,rad))
                anchor_counts[key]=len(anchors)
                if key in (36,54,72):
                    # The disjoint midpoint 25 ms either side of a grid boundary
                    # is at least 100 ms away from either training center.
                    mask=(ii>=hop)&(ii<len(media)-hop)&((ii%hop<rate//40)|(ii%hop>=hop-rate//40))
                    window=(ii>=hop-rate//40)&(ii<hop+rate//40)
                    assert not np.any((mask | window) & (abs((ii+hop//2)%hop-hop//2)>hop//2-half))
                    orig=media.astype('f8')/32768.
                    def raw(m):
                        return float(20*np.log10(np.linalg.norm(orig[m]-media_f[m])/np.linalg.norm(orig[m])))
                    source_metrics[key]={'anchors':len(anchors),'held_out_slices_signed_dbr':raw(mask),
                                         'independent_first_gap_signed_dbr':raw(window)}
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
 for v in ('original','bare','clockbasis'):
    a,_,receipt=render(v,ev,end)
    out[v]=a
    if not test_holdout:assert len(receipt)==1 and receipt[0][:2]==[49875,55] and receipt[0][3]==0.
 assert all(x.shape==out['original'].shape and np.isfinite(x).all() for x in out.values())
 return {k:{v:score(out[v],out['original'],i,j) for v in ('bare','clockbasis')} for k,i,j in windows}
scope0=[('onset_36',16384,18432),('held_36',24576,37634),('release_36',38434,47000),
        ('pending_54',49459,49875),('onset_54',49875,51923),('held_54',53971,81434),('release_54',82534,100000)]
scope1=[('onset_73',16384,18432),('held_73',24576,37634),('release_73',38434,47000)]
source_metrics={}
anchor_counts={}
assert pin(BIN)=="6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850"
assert hashlib.sha256(data).hexdigest()=="ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740"
ret={'scope':'no asset export, frozen source-time, 7-harmonic per-zone synthesis from disjoint SFZ raw training windows, magnitude/phase interpolated globally at true source timing (descriptors in memory only); current real compiled original receiver, projected common control/timing with all cold note events retained, no terminal phase/time/gain fit',
     'core_sha':pin(BIN),'auth_older_container_sha':hashlib.sha256(data).hexdigest(),
     'cube_full_source':both(events,104000,scope0),
     'blind_key73_source':both({16384:[(True,(73,86))],38434:[(False,(73,0))]},48000,scope1,True)}
ret['independent_raw_source_gaps']=source_metrics
ret['logical_coefficient_bytes_only']=30*sum(anchor_counts.values())
ret['count_source_time_anchors']=anchor_counts
print(json.dumps(ret,indent=2,allow_nan=False))
