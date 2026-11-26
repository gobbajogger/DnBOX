from pathlib import Path
import sys,hashlib
core,drv,asset,ao=map(Path,sys.argv[1:])
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
                        extern void release_trace(unsigned,unsigned,unsigned,unsigned,float,float);
                        release_trace(k,i,v->gate,lv->amp.stage,amp,step);
                        extern float shape_carrier(unsigned,unsigned,unsigned,float,float);
                        const float c=witness_sample(i,witness_notes[i],step,old);
                        x+=s->sj6_sub_gain*shape_carrier(k,i,v->gate,amp,c);''')]:
 assert s.count(old)==1
 s=s.replace(old,new)
core.write_text(s)
s=drv.read_text().replace('int main(int argc,char**argv)','__attribute__((visibility("default"))) int run_render(int argc,char**argv)')
needle='static float peak[2][2];'
assert s.count(needle)==1
s=s.replace(needle, """static float peak[2][2];
static unsigned begin;
extern void shape_setup(float,float,float,float);
extern void shape_block(unsigned,unsigned);
extern double witness_cursor(unsigned);
void release_trace(unsigned i,unsigned slot,unsigned gate,unsigned stage,float amp,float step){
  const unsigned at=begin+i;
  if(at>=38370 && at<39458)fprintf(stderr,"SUBREL %u %u %u %u %a %a %a\\n",at,slot,gate,stage,(double)amp,(double)step,witness_cursor(slot));
}""")
needle='    assert(original_runtime_render(&part,&work,l,r,frames)==ORIGINAL_RUNTIME_OK);'
assert s.count(needle)==1
s=s.replace(needle,'    begin=total;shape_block(begin,frames);\n'+needle)
needle='  prepare(bpm);'
assert s.count(needle)==1
s=s.replace(needle,needle+'\n  shape_setup(word[36],word[37],word[38],word[39]);')

drv.write_text(s)
a=asset.read_text();p='voice[i]=(struct voice){0}; ++resets;';assert a.count(p)==1
a=a.replace(p,'extern void shape_reset(unsigned); shape_reset(i); '+p)
ao.write_text(a+'\ndouble witness_cursor(unsigned i){ assert(i<6); return voice[i].pos; }\n')
