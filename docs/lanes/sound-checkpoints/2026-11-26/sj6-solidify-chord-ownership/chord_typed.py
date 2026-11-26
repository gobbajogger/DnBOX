from pathlib import Path
import sys
t=Path(sys.argv[1]);p=t/'suffix-driver.c';s=p.read_text()
q='int main(int argc,char**argv){'
s=s.replace(q,'''static void chord_trace(char *tag,unsigned at,unsigned key,unsigned value) {
 const dnbx_voice *d=&part.dsp.juno.synth.core.voice[0];
 fprintf(stderr,"%s %u %u %u %u %u",tag,at,key,value,d->note,d->gate);
 for(unsigned i=0;i<2;i++){
    const dnbx_layer_voice *l=d->layer+i;
    fprintf(stderr," %a %a %u %a",l->increment*44100.f,l->phase,l->amp.stage,l->amp.level);
 }
 fputc('\\n',stderr);
}
'''+q)
a='assert(original_runtime_key(&part,ORIGINAL_INPUT_USB,key,(v&&bid(205))?87u:v)==ORIGINAL_RUNTIME_OK);'
assert s.count(a)==1
s=s.replace(a,'chord_trace("pre",total,key,v);'+a+'chord_trace("post",total,key,v);')
a='assert(original_runtime_render(&part,&work,l,r,frames)==ORIGINAL_RUNTIME_OK);'
assert s.count(a)==1
s=s.replace(a,a+'''
    if(total>=49331 && total<50150 || total>=82400 && total<83250)chord_trace("render",total+frames,0,0);
''')
(t/'chord-driver.c').write_text(s)
