from pathlib import Path
import tempfile,subprocess,sys
old,new=map(Path,sys.argv[1:])
def between(s,a,b):return s[s.index(a):s.index(b,s.index(a))]
common=(old/'remote_synth_panel.inc').read_text()
find=between(common,'typedef struct { uint8_t id, kind, c; }','\n#include "remote_s1_panel.inc"')
c='''#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef struct { unsigned kind,a,b,c; } pref_t;
#define PR(k,a,b,c) ((pref_t){k,a,b,c})
#define T_303 2
#define R_SJ6CRUSH 97
'''+find
tail='''\nstatic const rd_panel_row rows[]={\n'''
for label,root in (('old',old),('new',new)):
 s=(root/'remote_sj6_panel.inc').read_text()
 seg=between(s,'        {107, R_SJ6CRUSH, 0},','\n#endif\n#if defined(DNBOX_137_HOST_WITNESS) && DNBOX_TYPED_CONTROLS_NATIVE_TEST && X0X_SJ6_OTA_NATIVE') 
 v=c+tail+seg+'''\n};
int main(void){
 pref_t p;
 for(unsigned v=0;v<2;v++){
  assert(rd_panel_position(rows,sizeof rows/sizeof *rows,v,107,&p)&&p.kind==R_SJ6CRUSH&&p.b==v&&p.c==0);
  assert(rd_panel_position(rows,sizeof rows/sizeof *rows,v,108,&p)&&p.kind==R_SJ6CRUSH&&p.b==v&&p.c==1);
#ifdef SJ6_RATE_CARRIER_WITNESS
  assert(rd_panel_position(rows,sizeof rows/sizeof *rows,v,109,&p)&&p.kind==R_SJ6CRUSH&&p.b==v&&p.c==2);
#else
  assert(!rd_panel_position(rows,sizeof rows/sizeof *rows,v,109,&p));
#endif
  assert(!rd_panel_position(rows,sizeof rows/sizeof *rows,v,110,&p));
 }
 puts("real typed row matcher: stable 107/108, guarded 109, negative adjacent and stock 109");
}
'''
 with tempfile.TemporaryDirectory() as t:
  src=Path(t)/'test.c';src.write_text(v);out=src.with_suffix('')
  flags=['-DX0X_HOST=1','-DSJ6_RATE_CARRIER_WITNESS=1'] if label=='new' else []
  subprocess.run(['cc','-std=c11','-Wall','-Werror','-O2',*flags,str(src),'-o',str(out)],check=True)
  subprocess.run([str(out)],check=True)
