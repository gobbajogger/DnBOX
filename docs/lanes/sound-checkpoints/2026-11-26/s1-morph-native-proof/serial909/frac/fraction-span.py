from pathlib import Path
import sys
p=Path(sys.argv[1])/'tree/firmware/src/dsp/drum909.c';t=p.read_text()
a='''        s->pos += (unsigned)m;
    } else {
        for (int i = 0; i < m; ++i) {'''
b='''        s->pos += (unsigned)m;
    } else if (m>0 && s->inc==1u && s->pos<len && (unsigned)(2*m+1)<len-s->pos &&
               (s->dk==s->pos || s->dk==s->pos+1u)) {
        /* At rate [1,2) there is one required new source sample per output,
         * plus one on carry. Prime if necessary, but don't decode beyond
         * the last rendered output (preserve persisted lookahead exactly). */
        uint32_t frac=s->frac, incf=s->incf, pos=s->pos;
        unsigned extra=s->dk==pos;
        int32_t h1=s->h1, h0=s->h0;
        float *y=bus->y;
        for (int i=0;i<m;++i) {
            if (extra) h1=d9_smp_decode(s,adpcm);
            h0=h1; h1=d9_smp_decode(s,adpcm);
            const float a=(float)h0*(1.0f/32768.0f);
            const float b=(float)h1*(1.0f/32768.0f);
            const float fr=(float)frac*(1.0f/4294967296.0f);
            const float v=a+(b-a)*fr;
            const uint32_t next=frac+incf;
            extra=next<frac;
            pos+=1u+extra;
            frac=next;
            y[i]=v*d9_env_tick(&s->out);
        }
        s->frac=frac; s->pos=pos; s->h0=h0; s->h1=h1;
    } else {
        for (int i = 0; i < m; ++i) {'''
assert t.count(a)==1
p.write_text(t.replace(a,b))
