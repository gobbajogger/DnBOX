"""Actual current loaded Sub receiver with old authorized assets kept in memory.
Signed harmonic attribution only, no preservation of raw data/learned product model."""
from pathlib import Path
import math
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('base,trace,commit=render(False)')[0]
a='    for n in q[\'buffers\']:'
assert s.count(a)==1
s=s.replace(a,'''    pre=np.zeros(148684,dtype='f4'); post=pre.copy(); at_pre=at_post=0
    def p_read(*_):
        nonlocal at_pre
        pre[now+at_pre]=e.s(0); at_pre+=1
    def v_read(*_):
        nonlocal at_post
        post[now+at_post]=e.s(1);at_post+=1
    e.uc.hook_add(UC_HOOK_CODE,p_read,begin=0x1003b78e0,end=0x1003b78e0)
    e.uc.hook_add(UC_HOOK_CODE,v_read,begin=0x1003b7934,end=0x1003b7934)
'''+a)
s=s.replace('        off=0; stage.append(d.render(n));now+=n','''        at_pre=at_post=0
        off=0; stage.append(d.render(n));now+=n
        assert at_pre==at_post and at_pre<=n''')
s=s.replace('if now>=57344:break','if now>=82534:break')
s=s.replace('return np.concatenate(stage,axis=1),tap,commit','return np.concatenate(stage,axis=1),tap,commit,pre[:now],post[:now]')
exec(compile(s,'authorized pinned source projection','exec'))
def harmonics(v,f,ix,sr=44100):
 tt=np.arange(len(v))/sr
 w=np.hanning(len(v))
 return np.array([np.dot(w*v,np.exp((-2j*np.pi*f*h)*tt)) for h in ix])/sum(w)*2
def suffix_progress(v):
 from scipy.signal import lfilter
 f=10/44100; w=2*np.pi*f; c=np.cos(w); alpha=np.sin(w)/2
 d=1+alpha
 hp=lfilter([(1+c)/(2*d),-(1+c)/d,(1+c)/(2*d)],[1,-2*c/d,(1-alpha)/d],v)
 crush=lfilter([1,-1],[1,-float.fromhex('0x1.fd70a4p-1')],hp)
 final=lfilter([1,-1],[1,-float.fromhex('0x1.ff7ceep-1')],crush)
 return {'after_known_hp10':hp,'after_connected_zero_crusher':crush,'after_known_final_dc':final}
def spectrum(v,f,sr=44100):
 # Small spectral peak search describes each independent received stream.
 # Signed comparisons retain the real cue and no use of this frequency
 # to modify candidate/source time, levels or source parameters.
 tt=np.arange(len(v))/sr
 w=np.hanning(len(v))*v
 fs=np.linspace(f*.985,f*1.015,129)
 p=np.array([abs(np.dot(w,np.exp(-2j*np.pi*i*tt))) for i in fs])
 est=fs[np.argmax(p)]
 h=harmonics(v,est,(1,2,3,5),sr)
 o=dict(frequency_hz=float(est),rms=float(np.sqrt(np.mean(v.astype('f8')**2))))
 for k,n in enumerate((2,3,5),1):
    z=h[k]/abs(h[0])*np.exp(-1j*n*np.angle(h[0]))
    o[f'h{n}']=dict(dbc=float(20*np.log10(abs(z))),phase_deg=float(np.angle(z,deg=True)))
 return o
x,trace,commit,pre,post=render(False)
assert commit[0][0]==49875
# After-commit body experiment already proved lead-in cannot simply be skipped,
# read it separately here only to identify static spectrum.
res={}
forward=suffix_progress(post)
for nm,lo,hi,f,key in [('first_hold',24576,37634,32.7032,36),('second_hold',53971,81434,97.9989,54)]:
 assert np.array_equal(x[0,lo:hi],post[lo:hi])
 rr={'frames':[lo,hi],'old_media_through_current_prefilter':spectrum(pre[lo:hi],f),
       'old_media_through_current_filter':spectrum(post[lo:hi],f),
       'installed_native':spectrum(reference[0,lo:hi],f),'installed_native_repeat':spectrum(repeat[0,lo:hi],f)}
 g=next(r for r in regions if int(r['pitch_keycenter'])==key)
 sr,med=pack[g['sample'].replace('\\','/').split('/')[-1]]
 fs=(440*2**((key-69)/12))*.5
 l,h=map(int,[g['loop_start'],g['loop_end']])
 rr['older_asset_body']=spectrum(med[l:min(h,l+sr)].astype('f8')/32768,fs,sr)
 rr['older_asset_start']=spectrum(med[:sr].astype('f8')/32768,fs,sr)
 rr.update({k:spectrum(v[lo:hi],f) for k,v in forward.items()})
 rr['asset_root']=key
 res[nm]=rr
print(json.dumps(dict(scope='Old authorized sample bytes only in emulator memory, actual current compiled receiver, held tap vs verified installed AU; spectral phase relative to H1, no waveform shift/fit, authored LFO graphs not replayed',core_sha=pin(BIN),asset_container_sha=hashlib.sha256(data).hexdigest(),native_pcm_sha=q['pcm_sha256'],windows=res),indent=2))
