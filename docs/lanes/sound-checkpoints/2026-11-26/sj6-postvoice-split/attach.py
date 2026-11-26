from pathlib import Path
import sys
j,drv,rt=map(Path,sys.argv[1:])
s=j.read_text()
n=' float hp_hz = s->hpf > 0.f ? 15+2000*s->hpf*s->hpf : 0.f;'
assert s.count(n)==1
s=s.replace(n,''' { extern void postvoice_signal(float*,float*,unsigned,float); postvoice_signal(left,right,frames,s->hpf); }
'''+n)
j.write_text(s)
s=drv.read_text()
p='int main(int argc,char**argv)'
assert s.count(p)==1
s=s.replace(p,'__attribute__((visibility("default"))) int run_render(int argc,char**argv)')
drv.write_text(s)

s=rt.read_text()
p='''#if X0X_SJ6_NATIVE_J60
            if(p->dsp.juno.chorus.j60.selected)'''
assert s.count(p)==1
s=s.replace('                juno6_j60_process(&p->dsp.juno.chorus,left,right,frames);', '''                {
                    extern int j60_probe(float*,float*,unsigned,unsigned);
                    if(!j60_probe(left,right,frames,p->dsp.juno.chorus.factory_mode))
                        juno6_j60_process(&p->dsp.juno.chorus,left,right,frames);
                }''')
s=s.replace(p,'''            {
                extern void physical_crusher(float*,float*,unsigned,int,float);
                physical_crusher(left,right,frames,p->dsp.juno.crush.active,p->dsp.juno.crush.step);
            }
'''+p)
p='''                right[i]*=gain;
            }
}'''
assert s.count(p)==1
s=s.replace(p,'''                right[i]*=gain;
            }
            { extern void physical_terminal(float*,float*,unsigned); physical_terminal(left,right,frames); }
}''')
rt.write_text(s)
