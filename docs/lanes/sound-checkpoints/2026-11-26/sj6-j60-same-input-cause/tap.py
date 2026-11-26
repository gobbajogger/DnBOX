from pathlib import Path
import sys,hashlib
rt,drv,o=map(Path,sys.argv[1:])
a=rt.read_text()
assert hashlib.sha256(rt.read_bytes()).hexdigest()=='c175a5d50a161fcc0d4eb7dade04b6c6205d2243bd241a768ff93902e0d63def'
needle='''            if(p->dsp.juno.chorus.j60.selected)
                juno6_j60_process(&p->dsp.juno.chorus,left,right,frames);
            else'''
assert a.count(needle)==1
a=a.replace(needle,'''            if(p->dsp.juno.chorus.j60.selected){
                extern void witness_j60(unsigned,const float*,const float*,unsigned);
                witness_j60(0,left,right,frames);
                juno6_j60_process(&p->dsp.juno.chorus,left,right,frames);
                witness_j60(1,left,right,frames);
            } else''')
(o/'runtime.c').write_text(a)
s=drv.read_text()
s=s.replace('static float peak[2][2];',r'''static FILE *tap[2],*partition;
void witness_j60(unsigned which,const float *l,const float *r,unsigned n){
    if(!which)assert(fwrite(&n,sizeof(n),1,partition)==1);
    for(unsigned i=0;i<n;i++){
        const float xy[]={l[i],r[i]};
        assert(fwrite(xy,sizeof(float),2,tap[which])==2);
    }
}
static float peak[2][2];''')
needle='  prepare(bpm);'
assert s.count(needle)==1
s=s.replace(needle, ''.join(f'  tap[{i}]=fopen("{o/"tap"}-{i}.f32","wb"); assert(tap[{i}]);\n' for i in (0,1)) +f'  partition=fopen("{o/"blocks.i32"}","wb");assert(partition);\n'+needle)
(o/'driver.c').write_text(s)
