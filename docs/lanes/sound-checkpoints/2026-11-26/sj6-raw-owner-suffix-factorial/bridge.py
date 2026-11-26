from pathlib import Path
import sys
p=Path('sj6-warm-postvoice-physical/blend.c')
s=p.read_text()
s=s.replace('static unsigned counts[3];','''static unsigned counts[3], stage_n[4];
static float stage[4][2][148684];
__attribute__((visibility("default"))) const float *stage_ptr(unsigned x){assert(x<4 && stage_n[x]==148684);return stage[x][0];}
static void tap(unsigned x,const float*l,const float*r,unsigned n){
 unsigned a=stage_n[x];assert(a+n<=148684);
 memcpy(stage[x][0]+a,l,n*sizeof(float));memcpy(stage[x][1]+a,r,n*sizeof(float));stage_n[x]+=n;
}''')
s=s.replace(' assert(!guard || take);',' memset(stage_n,0,sizeof stage_n);')
s=s.replace(' if(physical){\n  for',' tap(0,l,r,n);\n if(physical){\n  for')
s=s.replace(' at+=n;', ' tap(1,l,r,n);\n at+=n;')
s=s.replace(' if(!physical)return;\n assert',' if(!physical){tap(2,l,r,n);return;}\n assert')
s=s.replace(' counts[1]+=n;',' tap(2,l,r,n); counts[1]+=n;')
s=s.replace(' if(!physical)return;\n float *z',' tap(3,l,r,n);\n if(!physical)return;\n float *z')
Path(sys.argv[1]).write_text(s)
