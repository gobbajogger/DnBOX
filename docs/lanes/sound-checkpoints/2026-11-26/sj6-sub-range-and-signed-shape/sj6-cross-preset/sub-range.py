"""Apply original global range before compact Sub zone lookup and octave-down.
No oscillator or phase algorithm, gain, postfilter, or note lifecycle edits.
Only exact typed RANGE enum is supported. Out-of-range endpoints retain stock
until authentic sampler address/dispatch source is scoped.
"""
from pathlib import Path
import hashlib,sys
p,q=map(Path,sys.argv[1:])
b=p.read_bytes()
assert hashlib.sha256(b).hexdigest()=='d58f87a1f598fb894a184ccd57f543a1ec451ee13aea020b6eec9e92ca0da625'
s=b.decode()
a='''    if(s->sj6_sub_gain)
        for(unsigned i=0;i<DNBX_VOICES;++i) {
            unsigned note=s->voice[i].note;
            sj6_steps[i]=.5f*pitch_increment(s,note,0.f)*source_bend;
            sj6_harmonics[i]=sj6_sub_harmonic[sj6_sub_zone(note)];
        }'''
c='''    if(s->sj6_sub_gain) {
        float range;
        int shift=0;
        if(s->superj6_amp_range && dnbx_read_raw(s,86u,&range)==DNBX_OK) {
            if(range==0.f)shift=-12;
            else if(range==1.f)shift=12;
        }
        for(unsigned i=0;i<DNBX_VOICES;++i) {
            unsigned note=s->voice[i].note;
            const unsigned ranged=(unsigned)((int)note+shift);
            if(ranged<=127u)note=ranged;
            sj6_steps[i]=.5f*pitch_increment(s,note,0.f)*source_bend;
            sj6_harmonics[i]=sj6_sub_harmonic[sj6_sub_zone(note)];
        }
    }'''
assert s.count(a)==1
q.write_text(s.replace(a,c))
