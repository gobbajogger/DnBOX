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
def meas(x):
    x=x.astype('f8')
    assert np.isfinite(x).all()
    return dict(rms=float(np.sqrt(np.mean(x*x))),peak=float(np.max(abs(x))))
assert pin(BIN)=='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
assert hashlib.sha256(data).hexdigest()=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
x,trace,commit,pre,post=render(False)
assert commit[0][:2]==[49875,55]
cp=Path(sys.argv[1]); tape=np.fromfile(cp,'<f4').reshape(-1,6)
co=Path(sys.argv[2]); out=np.fromfile(co,'<f4').reshape(-1,2)
assert tape.shape==(148684,6) and out.shape==(148684,2)
rows={}
for name,a,b in [('first_hold',24576,37634),('second_hold',53971,81434)]:
    assert np.array_equal(x[0,a:b],post[a:b]) # one true original voice equals returned output
    ca=tape[a:b]
    row={k:meas(v) for k,v in dict(original_pre_iir=pre[a:b],original_post_iir=post[a:b],
                                 compact_carrier_with_fader=ca[:,0],compact_pre_iir_with_equivalent_vca=ca[:,1],
                                 compact_post_iir_vca=ca[:,2],compact_layer_sum=ca[:,3],compact_reducer_l=ca[:,4],
                                 compact_reducer_r=ca[:,5],compact_final=out[a:b,0],native_final=reference[0,a:b],
                                 native_repeat=repeat[0,a:b]).items()}
    assert np.array_equal(ca[:,2],ca[:,3])
    # True symmetric Sub mixer route, no approximation: rounded (layer/6)*.5
    assert np.array_equal((ca[:,3]*np.float32(1/6))*np.float32(.5),ca[:,4])
    assert np.array_equal(ca[:,4],ca[:,5])
    db=lambda u,v: float(20*np.log10(row[u]['rms']/row[v]['rms']))
    rows[name]={'frames':[a,b],'stage_levels':row,'compact_equiv_pre_vs_orig_pre_db':db('compact_pre_iir_with_equivalent_vca','original_pre_iir'),
                'compact_voice_vs_orig_voice_db':db('compact_post_iir_vca','original_post_iir'),
                'compact_actual_reducer_db':db('compact_reducer_l','compact_layer_sum'),
                'compact_vs_native_final_db':db('compact_final','native_final'),
                'compact_unity_after_reducer_only_vs_native_db':db('compact_final','native_final')-db('compact_reducer_l','compact_layer_sum')}
print(json.dumps(dict(scope='Same cue, absolute original isolated latest compiled Sub receiver with hash-matched old authorized sample media, versus actual compact with admitted typed projection; current physical projected receiver settings not equivalent to absent full live LFO graph. Compact pre-IIR VCA is explicitly counterfactual multiplication for measurement only, output unchanged. Last-column reducer algebra does not qualify matched downstream chain or limiter.',core=pin(BIN),asset=hashlib.sha256(data).hexdigest(),native=q['pcm_sha256'],compact_terminal=pin(co),commit=commit[0][:2],windows=rows),indent=2))
