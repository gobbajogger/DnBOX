from pathlib import Path
import sys,hashlib
p,src,o=map(Path,sys.argv[1:]); s=p.read_text()
# New clock/shape is identical in both arms; this is only the existing compact
# two-layer RNG advance on accepted distinct physical note-on; no phase reset.
a='static void sj6_vintage_attack('
assert s.count(a)==1
marker='\n#endif\n#if X0X_SJ6_VINTAGE_NATIVE || X0X_SJ6_SCALE_NATIVE'
assert s.count(marker)==1
s=s.replace(marker,'''
#if defined(X0X_HOST) && DNBX_HOST_VINTAGE_LEGATO
void witness_legato_vintage(dnbx_part *s) {
    if(s->superj6_amp_range && s->sj6_vintage_limit && s->voice[0].gate &&
       s->layer[0].params.waveform==0.f && s->layer[1].params.waveform==0.f) {
        for(unsigned l=0;l<DNBX_LAYERS;l++)sj6_vintage_attack(s,&s->voice[0],l);
    }
}
#endif
'''+marker)
p.write_text(s)
q=src/'juno6_factory.inc'; b=q.read_bytes()
assert hashlib.sha256(b).hexdigest()=='18615ef0dba599add05597eb843d428a4a67c286cfde71f55d037e4c4263f57c'
s=b.decode()
a='  const int continuing=s->core.voice[0].gate&&mono_has_keys(s);'
assert s.count(a)==1
s=s.replace(a,a+'''
#if defined(X0X_HOST) && DNBX_HOST_VINTAGE_LEGATO && X0X_SJ6_VINTAGE_NATIVE
  const int distinct=!mono_sounding_key(s,note);
#endif''')
a='   return dnbx_superj6_legato_voice(&s->core,note)==0;'
assert s.count(a)==1
s=s.replace(a,'''
#if defined(X0X_HOST) && DNBX_HOST_VINTAGE_LEGATO && X0X_SJ6_VINTAGE_NATIVE
   extern void witness_legato_vintage(dnbx_part*);
   if(distinct
#if X0X_SJ6_RANDOM5_6_NATIVE
     && !zero_on
#endif
   )witness_legato_vintage(&s->core);
#endif
'''+a)
(o/'factory.inc').write_text(s)
(o/'juno.c').write_text((src/'juno6.c').read_text().replace('#include "juno6_factory.inc"','#include "factory.inc"'))
