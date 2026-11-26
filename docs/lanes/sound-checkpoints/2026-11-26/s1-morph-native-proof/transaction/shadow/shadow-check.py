from pathlib import Path
import sys
p=Path(sys.argv[1])/'tree/firmware/src/dsp/drum909.c'; t=p.read_text()
a='extern void plat_cpu2_sync(void);'
assert t.count(a)==1
t=t.replace(a,a+'\nvolatile unsigned d9_verify_missed, d9_verify_good, d9_verify_state, d9_verify_pcm;\n')
a='''                *s=future->voice;
                bus.y=future->pcm;
                d9_emit_block(&bus,(int)future->frames);
                bus.y=y;
            } else
                d9_render_smp(s, &bus, n);'''
b='''                float *dry=bus.dry;
                bus.dry=0;
                d9_render_smp(s,&bus,n);
                if (__builtin_memcmp(s,&future->voice,sizeof(*s)))
                    ++d9_verify_state;
                else if (__builtin_memcmp(y,future->pcm,future->frames*sizeof(*y)))
                    ++d9_verify_pcm;
                else
                    ++d9_verify_good;
                bus.dry=dry;
                d9_emit_block(&bus,(int)future->frames);
            } else {
                if (pending && wanted==order[k]) ++d9_verify_missed;
                d9_render_smp(s, &bus, n);
            }'''
assert t.count(a)==1
p.write_text(t.replace(a,b))
