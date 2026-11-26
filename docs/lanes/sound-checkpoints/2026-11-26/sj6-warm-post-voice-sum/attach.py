from pathlib import Path
import sys
j,drv=map(Path,sys.argv[1:])
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
