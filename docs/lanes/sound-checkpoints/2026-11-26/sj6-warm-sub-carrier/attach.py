from pathlib import Path
import sys,hashlib
core,drv=map(Path,sys.argv[1:])
assert 'DNBX_HOST_SJ6_AUTHENTIC_CENTER' in core.read_text() and '#define SJ6_WAVE_PROBE 1' in core.read_text()
s=core.read_text()
for old,new in [
 ('if (!retrigger) memset(v,0,sizeof *v);','''if (!retrigger) {
        extern void witness_reset(unsigned); witness_reset(chosen);
        memset(v,0,sizeof *v);
    }'''),
 ('float sj6_steps[DNBX_VOICES];','float sj6_steps[DNBX_VOICES]; unsigned witness_notes[DNBX_VOICES];'),
 ('sj6_steps[i]=.5f*pitch_increment','witness_notes[i]=note; sj6_steps[i]=.5f*pitch_increment'),
 ('x+=s->sj6_sub_gain*sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]);','''
                        extern float witness_sample(unsigned,unsigned,float,float);
                        const float old= sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]);
                        x+=s->sj6_sub_gain*witness_sample(i,witness_notes[i],step,old);''')]:
 assert s.count(old)==1
 s=s.replace(old,new)
core.write_text(s)
s=drv.read_text().replace('int main(int argc,char**argv)','__attribute__((visibility("default"))) int run_render(int argc,char**argv)')
drv.write_text(s)
