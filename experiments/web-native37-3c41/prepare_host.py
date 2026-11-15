from pathlib import Path
import importlib.util,subprocess,json,hashlib
from native_html_core import verify_source
BASE=Path(__file__).resolve().parent;SRC=BASE/'source-restoration/native-3c41';OUT=BASE/'native-build';OUT.mkdir(exist_ok=True)
verify_source(SRC)
def body_edit(s,name,start='',end=''):
 import re
 m=re.search(r'(?:static(?: inline)?\s+)?(?:void|int32_t)\s+'+name+r'\([^;]*?\)\s*\{',s,re.S)
 if not m:raise ValueError('missing function '+name)
 a=m.end();depth=1;i=a
 while depth:
  if s[i]=='{':depth+=1
  elif s[i]=='}':depth-=1
  i+=1
 return s[:a]+start+s[a:i-1]+end+s[i-1:]
gfx=(SRC/'firmware/src/gfx.c').read_text();gfx=gfx.replace('#include "felucca_font.h"','#include "felucca_font.h"\n#include "'+str(BASE/'trace.h')+'"',1)
gfx=gfx.replace('cv_y1 = (int32_t)h;','cv_y1 = (int32_t)h;\n    tr_canvas(w,h,bg);',1)
gfx=body_edit(gfx,'cv_rect','\ntr_rect(x,y+cv_oy,w,h,c,cv_y0,cv_y1);\n')
gfx=body_edit(gfx,'cv_line','\ntr_line(x0,y0+cv_oy,x1,y1+cv_oy,c,cv_y0,cv_y1);tr_skip++;\n','\ntr_skip--;\n')
gfx=body_edit(gfx,'cv_pset','\ntr_point(x,y+cv_oy,c,cv_y0,cv_y1);\n')
gfx=body_edit(gfx,'cv_text_clip','\ntr_text(x,y+cv_oy,c,f,s,clip_left,clip_right,cv_y0,cv_y1);\n')
gfx=body_edit(gfx,'cv_blit','\ntr_commit(x,y,0);\n')
gfx=body_edit(gfx,'cv_blit_from','\ntr_commit(x,y,r0);\n')
(OUT/'gfx.c').write_text(gfx)
ui=(SRC/'firmware/src/app/ui.c').read_text();ui=body_edit(ui,'px_blend','\ntr_blend(x,y+cv_oy,c,a,cv_y0,cv_y1);\n')
for commit_function in ('cv_commit','cv_commit_rows','cv_commit_full_with_tiles'):
 ui=body_edit(ui,commit_function,'\ntr_commit(x,y,0);\n')
(OUT/'ui.c').write_text(ui)
host=(SRC/'host/x0x_host.c').read_text().replace('#include "../firmware/src/gfx.c"','#include "'+str(OUT/'gfx.c')+'"').replace('#include "../firmware/src/app/ui.c"','#include "'+str(OUT/'ui.c')+'"')
# Relocate other source-relative includes without changing their contents.
import re
host=re.sub(r'#include "(\.\./[^\"]+)"',lambda m:'#include "'+str((SRC/'host'/m[1]).resolve())+'"',host)
host='static void host_physical_frame(void);\n'+host
host=host.replace('            ui_frame();','            ui_frame();host_physical_frame();').replace('            ui_input_only();','            ui_input_only();host_physical_frame();')
(OUT/'host.c').write_text(host)
release=(SRC/'tests/host/original_release_ui_test.c').read_text().replace('#include "../../host/x0x_host.c"','#include "'+str(OUT/'host.c')+'"')
release=re.sub(r'#include "(\.\./[^\"]+)"',lambda m:'#include "'+str((SRC/'tests/host'/m[1]).resolve())+'"',release);(OUT/'release.c').write_text(release)
theme=(SRC/'tests/host/ui_theme_test.c').read_text().replace('#include "original_release_ui_test.c"','#include "'+str(OUT/'release.c')+'"').replace('int main(int argc,char **argv)','int theme_test_main(int argc,char **argv)')
theme=re.sub(r'#include "(\.\./[^\"]+)"',lambda m:'#include "'+str((SRC/'tests/host'/m[1]).resolve())+'"',theme);(OUT/'theme.c').write_text(theme)
spec=importlib.util.spec_from_file_location('native_build',SRC/'tools/build.py');b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
units=[SRC/'firmware/src'/p for p in b.O2_UNITS if p!='dsp/bass303.c']+[SRC/'firmware/src/app/factory_banks.c']+[SRC/p for p in [*b.ORIGINAL_SYNTH_UNITS,*b.ORIGINAL_SYNTH_BACKENDS]]
cmd=['cc','-std=c99','-O1','-w','-D_POSIX_C_SOURCE=200809L','-DX0X_HOST','-DBOARD_SMK37=1','-DX0X_LIVE_KEY_TEST=1','-DX0X_ORIGINAL_SYNTH=1','-DX0X_DEV_WOBBLE=0','-DX0X_WOBBLE=0','-DORIGINAL_USE_SOFT_FMA','-DMIAMI_FX_NO_HOST_FENV','-fno-fast-math','-ffp-contract=off']
for p in ['firmware/src','firmware/src/app','firmware/src/dsp','build/gen','experiments/original_synth','experiments/wobble']:cmd+=['-I'+str(SRC/p)]
cmd+=[str(BASE/'native_server.c'),*map(str,units),'-lm','-o',str(OUT/'native-server')]
(OUT/'compile-command.json').write_text(json.dumps(cmd,indent=2)+'\n')
with (OUT/'compile.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True)
print('Built isolated actual-source native-host observer; no guest/device firmware changes.')
