#!/usr/bin/env python3
"""Extend a source-verified prepared inventory capture with strict final-frame text audits.
Source is already instrumented by ui_inventory + ui_geometry. Never modifies it.
All partial native display slices retain their partial-composition scope.
"""
from pathlib import Path
import argparse,shutil,subprocess,os,json,hashlib
HERE=Path(__file__).resolve().parent

def once(s,a,b):
 if s.count(a)!=1:raise ValueError('nonunique extension hook '+a[:80])
 return s.replace(a,b,1)

def prepare(source,out):
 work=out/'source';shutil.copytree(source,work,symlinks=True,ignore=shutil.ignore_patterns('.git','build','__pycache__'))
 p=work/'tests/ui_geometry/capture.c';s=p.read_text();s='static unsigned f2_extra_enabled,f2_layer_active,f2_layer_generation; static int f2_layer_rect[4];\n'+s
 s=once(s,'static void walkshot(', (HERE/'extra-audit.inc').read_text()+'\nstatic void walkshot(')
 s=once(s,'    retained_fixture_main();','f2_extra_start:\n    f2_extra_enabled=1;f2_reset();retained_fixture_main();')
 s=once(s,'    unsigned total=0;','    unsigned total=0;\n    if(getenv("F2_EXTRAS_ONLY"))goto f2_extra_start;')
 p.write_text(s)
 p=work/'firmware/src/gfx.c';s=p.read_text()
 s=once(s,'int x,y; unsigned col; } geo_label;','int x,y; unsigned col,layer; } geo_label;')
 s=once(s,'geo_label *e=&geo_tile[geo_ntile++];e->f=f;', 'geo_label *e=&geo_tile[geo_ntile++];e->layer=f2_layer_generation;e->f=f;')
 s=once(s,'static void geo_blit(int x,int y) {\n if(!geo_enabled)return;', '''static int32_t text_w(const felucca_font_t *f,const char *s);
static void geo_blit(int x,int y) {
 if(!geo_enabled)return;
 if(f2_extra_enabled) {
  unsigned keep=0;
  for(unsigned j=0;j<geo_nframe;j++) {
   geo_label *e=&geo_frame[j];
   int right=e->x+text_w(e->f,e->text)+2*e->f->pad;
   if(e->x>=x && e->y>=y && right<=x+(int)cv_w && e->y+e->f->h<=y+(int)cv_h)continue;
   geo_frame[keep++]=*e;
  }
  geo_nframe=keep;
 }''')
 s=once(s,'static void f1_reset(void){','static void f2_reset(void);\nstatic void f1_reset(void){f2_reset();');p.write_text(s)
 p=work/'tests/host/smc_ui_test.c';s=p.read_text();a='static void shot(const char *name)\n{';s=once(s,a,'static void f2_extra_audit(const char *screen);\n'+a+'\n f2_extra_audit(name);');p.write_text(s)
 p=work/'firmware/src/app/ui.c';s=p.read_text()
 s=once(s,'        box(bx, by, bw, bh, C_BLACK);','''        box(bx, by, bw, bh, C_BLACK);
        if(f2_extra_enabled && ui.touched==4) {
          f2_layer_active=1;f2_layer_generation=1;
          f2_layer_rect[0]=bx;f2_layer_rect[1]=MAIN_Y+cv_oy+by;
          f2_layer_rect[2]=bx+bw;f2_layer_rect[3]=MAIN_Y+cv_oy+by+bh;
        }''');p.write_text(s)
 return work

def run(source,out,smc_fix=False):
 out.mkdir(parents=True,exist_ok=False);work=prepare(source,out)
 if smc_fix:
  p=work/'firmware/src/app/smc_mixer.c';s=p.read_text()
  s=once(s,'            if (k.kind == R_FREEPLAY) label = "FREE";','            if (k.kind == R_FREEPLAY) label = "FREE";\n            if (k.kind == R_BPATSND) label = "SOUND";')
  s=once(s,'                text_c(x + 19, text_y, font, caption, c);','                text_c(x + 18, text_y, font, caption, c);');p.write_text(s)
 env={k:v for k,v in os.environ.items() if 'MUTATE' not in k};env.update(SMC_UI_OUT=str(out/'host'),SMC_UI_TEST_FILE='tests/ui_geometry/capture.c')
 with (out/'positive.log').open('w') as log:p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT)
 (out/'positive.exit').write_text(str(p.returncode)+'\n')
 rows=[json.loads(s[9:]) for s in (out/'positive.log').read_text().splitlines() if s.startswith('F2_EXTRA ')]
 findings=sum(r['findings'] for r in rows);not_run=[r for r in rows if not r['audited']]
 report={'renderer_exit':p.returncode,'frames':len(rows),'audited_texts':sum(r['audited'] for r in rows),'findings':findings,'zero_text_frames':not_run,'frames_detail':rows,'status':'PASS' if p.returncode==0 and rows and findings==0 and not not_run else 'FAIL','scope':'Opaque text-mask geometry only; separate source inventory proves reachable renderers. Does not certify unobserved branches, control semantics, glyph/panel geometry or hardware tails.'}
 (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k!='frames_detail'}))
 # Runtime damage is applied inside the final-frame oracle only for extra frames.
 env.update(F2_EXTRA_MUTATE='1',SMC_UI_SHOTS=str(out/'negative-shots'));(out/'negative-shots').mkdir()
 with (out/'negative.log').open('w') as log:q=subprocess.run([str(out/'host/test')],cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT)
 neg=[json.loads(s[9:]) for s in (out/'negative.log').read_text().splitlines() if s.startswith('F2_EXTRA ')]
 nr={'runtime_exit':q.returncode,'frames':len(neg),'findings':sum(r['findings'] for r in neg),'intended_marker':'F2_EXTRA_FINDING','rejected_by_verifier':any(r['findings'] for r in neg)}
 (out/'negative-receipt.json').write_text(json.dumps(nr,indent=2)+'\n');return 0
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepared-source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--smc-fix',action='store_true');a=p.parse_args();raise SystemExit(run(a.prepared_source.resolve(),a.output.resolve(),a.smc_fix))
