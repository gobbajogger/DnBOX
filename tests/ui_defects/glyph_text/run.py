#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Real glyph/text ink ownership observer. No complete visual gate claim."""
import argparse,hashlib,json,os,re,shutil,subprocess,types
from pathlib import Path

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def unique(s,a):
 if s.count(a)!=1:raise ValueError('Expected unique anchor: '+a)
def replace(s,a,b):unique(s,a);return s.replace(a,b,1)
def instrument(gfx,ui,glyph,bounds):
 gfx,ui,dims=bounds.instrument(gfx,ui)
 a='static void cv_begin(uint32_t w, uint32_t h, uint16_t bg)\n{'
 gfx=replace(gfx,a,'''static unsigned fwui_glyph_hit_calls,fwui_glyph_hit_pixels;
static unsigned char fwui_glyph_mask[240*138];
static uint16_t fwui_glyph_before[240*138],fwui_glyph_after[240*138];
'''+a+'\n    memset(fwui_glyph_mask,0,sizeof fwui_glyph_mask);')
 a='    fwui_text_calls++;fwui_text_pending=1;'
 gfx=replace(gfx,a,'    unsigned glyph_hits=0;\n'+a)
 a='                a = (gx & 1u) ? (a & 15u) : (a >> 4);'
 gfx=replace(gfx,a,a+'''
                if(a && px>=clip_left && px<clip_right && (uint32_t)px<cv_w && py>=cv_y0 && py<cv_y1 && fwui_glyph_mask[(uint32_t)py*cv_w+(uint32_t)px] && fwui_glyph_after[(uint32_t)py*cv_w+(uint32_t)px]==cv_px[(uint32_t)py*cv_w+(uint32_t)px])glyph_hits++;
''')
 a='    return x;\n}\n\nstatic int32_t cv_text('
 gfx=replace(gfx,a,'    if(glyph_hits){fwui_glyph_hit_calls++;fwui_glyph_hit_pixels+=glyph_hits;}\n'+a)
 glyph=replace(glyph,'static UI_DRAW void dv_glyph(','static UI_DRAW void dv_glyph_original(')
 a='static UI_DRAW void dv_value_text('
 glyph=replace(glyph,a,'''static void dv_glyph(pref_t r,float ev,float ox,float oy,float sx,float sy,uint16_t col)
{
    if(cv_w*cv_h>240*138)abort();
    memcpy(fwui_glyph_before,cv_px,cv_w*cv_h*sizeof(uint16_t));
    dv_glyph_original(r,ev,ox,oy,sx,sy,col);
    for(unsigned i=0;i<cv_w*cv_h;i++)if(fwui_glyph_before[i]!=cv_px[i]){fwui_glyph_mask[i]=1;fwui_glyph_after[i]=cv_px[i];}
}
'''+a)
 return gfx,ui,glyph

def load_captured(code,expected,filename):
 if hashlib.sha256(code).hexdigest()!=expected:raise ValueError('Observer helper hash mismatch')
 module=types.ModuleType('captured_bounds');module.__file__=str(filename)
 exec(compile(code,str(filename),'exec'),module.__dict__)
 return module

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--source',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);args=ap.parse_args()
 source=args.source.resolve();out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
 results=[];first=None
 for label in ('current','prior-placement'):
  dest=out/label;dest.mkdir();frozen=dest/'source';frozen.mkdir()
  for n in ('firmware','host','tests','tools','assets','experiments'):shutil.copytree(source/n,frozen/n,symlinks=True,ignore=shutil.ignore_patterns('__pycache__'))
  inputs={str(p.relative_to(frozen)):sha(p) for p in frozen.rglob('*') if p.is_file()};(dest/'inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
  if first is None:first=inputs
  elif inputs!=first:raise ValueError('Source changed between cases')
  helper='tests/ui_defects/text_bounds/run.py';captured=(frozen/helper).read_bytes();bounds=load_captured(captured,inputs[helper],frozen/helper)
  paths=['firmware/src/gfx.c','firmware/src/app/ui.c','firmware/src/app/ui_drum_voice.inc'];original=[(frozen/p).read_text() for p in paths]
  if original[2].count('y + ah - 16 - off')!=3:raise ValueError('Expected reviewed active-bank value placement')
  if label=='prior-placement':original[2]=original[2].replace('y + ah - 16 - off','y + ah - 20 - off')
  modified=instrument(*original,bounds)
  for p,s in zip(paths,modified):(frozen/p).write_text(s)
  pins={p:sha(frozen/p) for p in paths}
  def verify():
   changed={p for p,h in inputs.items() if sha(frozen/p)!=h}
   if changed!=set(paths) or any(sha(frozen/p)!=h for p,h in pins.items()):raise ValueError('Instrumented source drift')
  verify();env=os.environ.copy();env.update(SMC_UI_TEST_FILE='tests/host/fwui_glyph_layout_test.c',SMC_UI_OUT=str(dest/'build'))
  with (dest/'run.log').open('w')as log:r=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=frozen,env=env,stdout=log,stderr=subprocess.STDOUT)
  verify();rows=re.findall(r'^GLYPH_LAYOUT states=(\d+) bad=(\d+) checks=(\d+) outside=(\d+)$',(dest/'run.log').read_text(),re.M)
  if r.returncode or len(rows)!=1:raise ValueError('Producer failed: '+label)
  states,bad,checks,outside=map(int,rows[0]);results.append(dict(case=label,states=states,statesWithIntersection=bad,checks=checks,logicalOutside=outside,inputManifestSHA256=sha(dest/'inputs.json'),instrumented=pins,logSHA256=sha(dest/'run.log')))
 if results[0]['states']!=7434 or results[1]['states']!=7434 or results[0]['statesWithIntersection'] or not results[1]['statesWithIntersection'] or any(x['logicalOutside'] for x in results):raise ValueError('Regression expectation failed')
 report=dict(schema='dnbox-glyph-text-ownership/1',coverageComplete=False,results=results,scope='Factory other dependencies, all legal integer values of 63 current visible drum identities, idle settled selected SOUND pages. Pixel ownership uses actual glyph before/after RGB565 and requires unchanged produced color immediately before text paint. No all combinations, animation, complete inventory or F2 claim.')
 (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
if __name__=='__main__':main()
