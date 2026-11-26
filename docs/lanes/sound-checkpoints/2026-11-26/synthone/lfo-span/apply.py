from pathlib import Path
from hashlib import sha256
from difflib import unified_diff
import sys
target=Path(sys.argv[1]); ds='firmware/src/dsp/s1c/'
expected={
's1u.c':('7fa9df89ede499229f67d267cbfd58914a6a5b3deb41725db96b14d1cab7356e','a2028f11959ee09c3ae61e61e64476af4d52cabf0905b2ae4ef443924a69eddd'),
's1u.h':('6036f8086a3d4ebaf2c7a7d007ab525e94551a7d28bdf23fee931c4396e663d6','95221437fc4b0346693f6b54113ca5f093d984fec066f6aca3831c9ed52e3873'),
's1c.c':('6ab0d50a64b620d4649645a77100510695a5b72b143b2f7e670fc56acf1110cc','f9d255bd4a380450408e4b9cf74f8afb2c7060b9de722115887fb5cbd94820c2')}
def replace(s,a,b):
    assert s.count(a)==1
    return s.replace(a,b)
read={f:(target/ds/f).read_text() for f in expected}
new=dict(read)
for f in ('s1u.c','s1u.h'):
 n='smooth_c2' if f.endswith('.h') else 'c2'
 new[f]=replace(new[f],f'float rate_hz, int shape, float amount, float {n}, float sr)',f'float ph_step, int shape, float amount, float {n})')
new['s1u.c']=replace(new['s1u.c'],'l->ph += rate_hz / sr;','l->ph += ph_step;')
s=new['s1c.c']
s=replace(s,'    for (f = 0; f < n; f++) {\n#if S1C_FAST','''#if S1C_CSM && S1C_NO_SEQ
    /* L1R/L2R are not smoothed in this configuration. Program and tempo
     * ingress are serialized by the render owner outside this span. */
    const float lfo_step1 = P(L1R) / s->sr, lfo_step2 = P(L2R) / s->sr;
#endif
    for (f = 0; f < n; f++) {
#if S1C_FAST''')
old=''.join(f'        s1u_lfo_tick(&s->lfo[{i-1}], P(L{i}R), (int)P(L{i}I), P(L{i}A), lc2, s->sr);\n' for i in (1,2))
branch=lambda step:''.join(f'        s1u_lfo_tick(&s->lfo[{i-1}], {step(i)}, (int)P(L{i}I), P(L{i}A), lc2);\n' for i in (1,2))
new['s1c.c']=replace(s,old,'#if S1C_CSM && S1C_NO_SEQ\n'+branch(lambda i:f'lfo_step{i}')+'#else\n'+branch(lambda i:f'P(L{i}R) / s->sr')+'#endif\n')
digest=lambda s:sha256(s.encode()).hexdigest()
for f,(oldh,newh) in expected.items():
    assert digest(read[f])==oldh and digest(new[f])==newh,(f,digest(read[f]),digest(new[f]))
for f in expected:
    (target/ds/f).write_text(new[f])
    print(''.join(unified_diff(read[f].splitlines(True),new[f].splitlines(True),fromfile='a/'+ds+f,tofile='b/'+ds+f)),end='')
