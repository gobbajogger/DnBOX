from pathlib import Path
import sys
base=Path(sys.argv[1]); kind=sys.argv[2]; here=Path(__file__).parent
s=(base/'ab-latest-headless/firmware/src/dsp/s1c/s1c.c').read_text()
a='        s->da[0] += dL; s->da[1] += dR;'
assert s.count(a)==1
# Unchanged number of stored values, centering, and reconstruction. A rectangular
# centered sub-window has unity DC and leaves the comb feedback untouched.
if kind!='stock':
 div=int(kind)
 s=s.replace(a,f'''        const int skip=(dd-dd/{div})/2;
        const int keep=dd-2*skip;
        if(s->dk>=skip && s->dk<dd-skip) {{s->da[0] += dL; s->da[1] += dR;}}''')
 s=s.replace('float iL = s->da[0] / (float)dd, iR = s->da[1] / (float)dd;', 'float iL = s->da[0] / (float)keep, iR = s->da[1] / (float)keep;')
if len(sys.argv)>3:
 a=s.index('        float dm = P(DLMIX) * P(DLON);')
 b=s.index('        /* reverb:',a)
 s=s[:a]+'''        outl[f]=oL;outr[f]=oRR;continue;
        }
'''+s[b:]
(here/'firmware/src/dsp/s1c/s1c.c').write_text(s)
