#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Observe logical screen text ink bounds; not band visibility or complete geometry."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def unique(s, anchor):
    if s.count(anchor) != 1:
        raise ValueError('Expected unique anchor: ' + anchor)

def instrument(gfx, ui):
    dims = {}
    for name in ('HDR_H', 'MAIN_Y', 'BAND_H', 'KNOB_Y', 'KNOB_H'):
        values = re.findall(r'^#define '+name+r' (\d+)\b', ui, re.M)
        if len(values) != 1:
            raise ValueError('Unrecognized source strip layout: '+name)
        dims[name] = int(values[0])
    if dims != dict(HDR_H=20, MAIN_Y=20, BAND_H=69, KNOB_Y=158, KNOB_H=82):
        raise ValueError('Changed strip layout requires reviewed mapping')
    unique(gfx,'static int32_t cv_text_clip(')
    anchor = 'static void cv_begin('
    unique(gfx, anchor)
    prefix = '''static unsigned long long fwui_text_calls, fwui_text_ink, fwui_text_outside, fwui_text_explicit_clip, fwui_text_commits;
static unsigned fwui_text_pending;
static int fwui_text_origin(unsigned h) {
    if(h==20)return 0;if(h==138)return 20;if(h==82)return 158;
    fprintf(stderr,"UNSUPPORTED_TEXT_CANVAS %u\\n",h);abort();
}
static int fwui_text_logical_outside(int x,int y,unsigned h) {
    int logical_y=y+fwui_text_origin(h);
    return x<0 || x>=240 || logical_y<0 || logical_y>=240;
}
'''
    prefix += """static void fwui_text_commit_origin(unsigned x,unsigned y) {
    if(cv_w!=240 || x!=0 || y!=(unsigned)fwui_text_origin(cv_h)) {
        fprintf(stderr,"TEXT_COMMIT_ORIGIN_MISMATCH\\n");abort();
    }
    fwui_text_commits++;fwui_text_pending=0;
}
"""
    gfx = gfx.replace(anchor, prefix+anchor)
    begin = 'static void cv_begin(uint32_t w, uint32_t h, uint16_t bg)\n{'
    unique(gfx,begin)
    gfx=gfx.replace(begin,begin+'\n    if(fwui_text_pending){fprintf(stderr,"TEXT_CANVAS_NOT_COMMITTED\\n");abort();}')
    for function in ('cv_commit','cv_commit_rows','cv_commit_full_with_tiles'):
        pattern=r'(static void '+function+r'\([^;{}]*\)\s*\{)'
        if len(re.findall(pattern,ui))!=1:
            raise ValueError('Expected unique actual commit route: '+function)
        ui=re.sub(pattern,lambda m:m[1]+'\n    fwui_text_commit_origin(x,y);',ui,count=1)
    anchor = '    uint32_t r = c >> 11, g = (c >> 5) & 63u, b = c & 31u;'
    unique(gfx, anchor)
    gfx = gfx.replace(anchor, '''    fwui_text_calls++;fwui_text_pending=1;
    int fwui_full_clip=clip_left==0 && clip_right==(int32_t)cv_w;
    if(!fwui_full_clip)fwui_text_explicit_clip++;
    if(fwui_full_clip && cv_w!=240){fprintf(stderr,"UNSUPPORTED_TEXT_WIDTH\\n");abort();}
'''+anchor)
    anchor = '                a = (gx & 1u) ? (a & 15u) : (a >> 4);'
    unique(gfx, anchor)
    gfx = gfx.replace(anchor, anchor+'''
                if(a && fwui_full_clip) {
                    fwui_text_ink++;
                    if(fwui_text_logical_outside(px,py,cv_h))fwui_text_outside++;
                }
''')
    return gfx, ui, dims

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--source',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    source=args.source.resolve();out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
    frozen=out/'source';frozen.mkdir()
    for name in ('firmware','host','tests','tools','assets','experiments'):
        shutil.copytree(source/name,frozen/name,symlinks=True,ignore=shutil.ignore_patterns('__pycache__'))
    inputs={str(p.relative_to(frozen)):sha(p) for p in frozen.rglob('*') if p.is_file()}
    (out/'source-inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    gfx=frozen/'firmware/src/gfx.c';ui=frozen/'firmware/src/app/ui.c'
    text,instrumented_ui,dims=instrument(gfx.read_text(),ui.read_text());gfx.write_text(text);ui.write_text(instrumented_ui)
    results=[]
    for label,fixture in [('ordinary','ui_screen_inventory.c'),('lists','ui_overlay_inventory.c')]:
        target=frozen/'tests/host'/fixture;text=target.read_text()
        unique(text,'return 0;')
        # Existing fixture coverage assertions remain; do not publish new raw captures.
        text=text.replace('shot(name);','(void)name;')
        text=text.replace('return 0;', '''CHECK(fwui_text_calls>0 && fwui_text_ink>0 && fwui_text_commits>0 && !fwui_text_pending);
    CHECK(fwui_text_logical_outside(-1,0,20));CHECK(fwui_text_logical_outside(240,0,20));
    CHECK(fwui_text_logical_outside(0,82,82));CHECK(!fwui_text_logical_outside(0,81,82));
    CHECK(!fwui_text_logical_outside(23,-118,82));CHECK(!fwui_text_logical_outside(23,20,138));
    printf("TEXT_BOUNDS %llu %llu %llu %llu %llu\\n",fwui_text_calls,fwui_text_ink,fwui_text_outside,fwui_text_explicit_clip,fwui_text_commits);
    return 0;''')
        target.write_text(text)
        env=os.environ.copy();env.update(SMC_UI_TEST_FILE='tests/host/'+fixture,SMC_UI_OUT=str(out/label))
        with (out/(label+'.log')).open('w') as log:
            run=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=frozen,env=env,stdout=log,stderr=subprocess.STDOUT)
        log=(out/(label+'.log')).read_text()
        rows=re.findall(r'^TEXT_BOUNDS (\d+) (\d+) (\d+) (\d+) (\d+)$',log,re.M)
        if run.returncode or len(rows)!=1:
            raise ValueError('Producer failed; retain '+label+'.log')
        calls,ink,outside,explicit,commits=map(int,rows[0])
        results.append(dict(domain=label,calls=calls,decodedInkSamples=ink,logicalOutsideSamples=outside,explicitClipCallsExcluded=explicit,actualOriginChecks=commits,screenRecords=len(re.findall(r'^SCREEN ',log,re.M))))
    changed={p for p,h in inputs.items() if sha(frozen/p)!=h}
    expected={'firmware/src/gfx.c','firmware/src/app/ui.c','tests/host/ui_screen_inventory.c','tests/host/ui_overlay_inventory.c'}
    if changed!=expected:raise ValueError('Unexpected source modifications: '+str(changed))
    report=dict(schema='dnbox-logical-text-bounds/1',coverageComplete=False,sourceInputManifestSHA256=sha(out/'source-inputs.json'),stripMapping=dims,domains=results,instrumented={p:sha(frozen/p) for p in sorted(expected)},limits='Logical 240x240 ink bounds only; repeated band ink samples are not unique pixels. Explicit clips excluded. Does not prove band visibility, all text, overlap, content bounds, full inventory or visual acceptance.')
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
if __name__=='__main__':main()
