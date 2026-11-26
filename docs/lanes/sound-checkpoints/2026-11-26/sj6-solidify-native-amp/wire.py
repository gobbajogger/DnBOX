from pathlib import Path
import sys
g,o=map(Path,sys.argv[1:])
s=(g/'compose.c').read_text()
def repl(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
repl('    const float osc_gain[]={gain1,gain2};','''    const float osc_gain[]={gain1,gain2};
    extern int sj6_amp_diagnostic_begin(const dnbx_part*,unsigned);
    extern float sj6_amp_diagnostic_value(unsigned,unsigned);
    extern int sj6_amp_diagnostic_live(unsigned);
    int host_amp=sj6_amp_diagnostic_begin(s,n);
''')
repl('            if (!voice_active(v)) continue;','            if (!voice_active(v) && !(host_amp && !i && sj6_amp_diagnostic_live(k))) continue;')
repl('                float amp=envelope_sample(&lv->amp,&p->amp,s->sample_rate,w);','''                float amp=envelope_sample(&lv->amp,&p->amp,s->sample_rate,w);
                if(host_amp && !i)amp=sj6_amp_diagnostic_value(l,k);''')
(o/'typed-core.c').write_text(s)
s=(g/'suffix-driver.c').read_text()
repl('static void select_suffix(void) {','#include "host-amp.inc"\nstatic void select_suffix(void) {')
repl('  prepare(bpm); select_suffix();','  prepare(bpm); select_suffix(); amp_open();')
a='assert(original_runtime_key(&part,ORIGINAL_INPUT_USB,key,(v&&bid(205))?87u:v)==ORIGINAL_RUNTIME_OK);'
repl(a,'unsigned old=part.dsp.juno.synth.core.voice[0].gate;'+a+'amp_edge(old);')
a='    assert(original_runtime_render(&part,&work,l,r,frames)==ORIGINAL_RUNTIME_OK);'
repl(a,'    amp_span(frames);\n'+a+'\n    amp_checked(frames);')
repl('  assert(!fclose(pre));','  fprintf(stderr,"amp_admit=%d ons=%u offs=%u words=%u\\n",amp_scope,amp_on_count,amp_off_count,amp_words);\n  assert(!fclose(pre));')
(o/'driver.c').write_text(s)
