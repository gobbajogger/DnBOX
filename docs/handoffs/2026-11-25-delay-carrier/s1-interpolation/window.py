from pathlib import Path
import sys
base=Path(sys.argv[1]); kind=sys.argv[2]; here=Path(__file__).parent
s=(base/'ab-latest-headless/firmware/src/dsp/s1c/s1c.c').read_text()
a = "float u = (float)s->dk / (float)dd;"
assert s.count(a) == 1
curves={"stock":"", "cubic":"u=u*u*(3.0f-2.0f*u);", "cosine":"u=0.5f*(1.0f-cosf(3.14159265358979323846f*u));"}
s=s.replace(a,a+curves[kind])
if len(sys.argv)>3:
 a=s.index('        float dm = P(DLMIX) * P(DLON);')
 b=s.index('        /* reverb:',a)
 s=s[:a]+'''        outl[f]=oL;outr[f]=oRR;continue;
        }
'''+s[b:]
(here/'firmware/src/dsp/s1c/s1c.c').write_text(s)
