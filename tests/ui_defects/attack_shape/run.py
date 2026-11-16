#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Actual current/historical Attack span contract; no full UI/device acceptance."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def unique(s,a):
    if s.count(a)!=1:raise ValueError('Expected unique anchor: '+a)
def attack_block(s):
    a='    if (role == DG_ATTACK) {';z='    if (role == DG_DECAY || role == DG_NTAIL)'
    unique(s,a);unique(s,z);start=s.index(a);end=s.index(z)
    if end<=start:raise ValueError('Reordered Attack source')
    return start,end,s[start:end]
def instrument_header(s):
    old='''static void cv_poly_aa(const float *xy,int n,uint16_t col)
{
    for(int i=1;i<n;i++)cv_line_aa(xy[2*i-2],xy[2*i-1],xy[2*i],xy[2*i+1],col);
}'''
    unique(s,old)
    before='''static unsigned fwui_attack_recording,fwui_attack_calls,fwui_attack_count;
static float fwui_attack_xy[80];
'''
    wrapper='''
static void cv_poly_aa(const float *xy,int n,uint16_t col) {
    if(fwui_attack_recording) {
        if(n<0||n>40){fprintf(stderr,"INVALID_ATTACK_PATH\\n");abort();}
        fwui_attack_calls++;fwui_attack_count=(unsigned)n;
        memcpy(fwui_attack_xy,xy,(size_t)n*2*sizeof(float));
    }
    cv_poly_aa_actual(xy,n,col);
}
'''
    return s.replace(old,before+old.replace('cv_poly_aa(', 'cv_poly_aa_actual(')+wrapper)
def main():
    p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    source=a.source.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    fixture=source/'tests/ui_defects/attack_shape';old_file=fixture/'historical-f254-ui_drum_voice.inc';old=old_file.read_bytes()
    receipt=json.loads((fixture/'historical-receipt.json').read_text())
    blob=hashlib.sha1(b'blob '+str(len(old)).encode()+b'\0'+old).hexdigest()
    if receipt['commit']!='f254bf2c890623d6d543ed3b874ea225117f1290' or receipt['sha']!=blob or blob!='7b573423e2291523b747d21b127055f19d26d8d6':raise ValueError('Historical source identity mismatch')
    _,_,historic=attack_block(old.decode());results=[];original_inputs=None
    for label in ['current','historical-attack-composition']:
        case=out/label;case.mkdir();frozen=case/'source';frozen.mkdir()
        for name in ['firmware','host','tests','tools','assets','experiments']:
            shutil.copytree(source/name,frozen/name,symlinks=True,ignore=shutil.ignore_patterns('__pycache__'))
        inputs={str(f.relative_to(frozen)):sha(f) for f in frozen.rglob('*') if f.is_file()}
        if original_inputs is not None and inputs!=original_inputs:raise ValueError('Original input manifests differ between cases')
        original_inputs=inputs
        (case/'source-inputs.json').write_text(json.dumps(inputs,indent=2,sort_keys=True)+'\n')
        header=frozen/'firmware/src/app/ui_glyph_renderer.h';header.write_text(instrument_header(header.read_text()))
        expected={'firmware/src/app/ui_glyph_renderer.h'}
        if label!='current':
            target=frozen/'firmware/src/app/ui_drum_voice.inc';text=target.read_text();start,end,_=attack_block(text);target.write_text(text[:start]+historic+text[end:]);expected.add('firmware/src/app/ui_drum_voice.inc')
        changed={name for name,h in inputs.items() if sha(frozen/name)!=h}
        if changed!=expected:raise ValueError('Unexpected changes: '+str(changed))
        instrumented_before={name:sha(frozen/name) for name in sorted(expected)}
        env=os.environ.copy();env.update(SMC_UI_TEST_FILE='tests/host/ui_attack_shape_test.c',SMC_UI_OUT=str(case/'host'))
        with (case/'run.log').open('w') as log:
            run=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=frozen,env=env,stdout=log,stderr=subprocess.STDOUT)
        changed_after={name for name,h in inputs.items() if sha(frozen/name)!=h}
        if changed_after!=expected or any(sha(frozen/name)!=h for name,h in instrumented_before.items()):raise ValueError('Source changed during compile/render')
        text=(case/'run.log').read_text();rows=[json.loads(s) for s in re.findall(r'^ATTACK_RESULT (.*)$',text,re.M)]
        summary=re.findall(r'^ATTACK_SUMMARY states=(\d+) failures=(\d+) checks=(\d+)$',text,re.M)
        expected_failures=0 if label=='current' else 24
        if len(rows)!=48 or len(summary)!=1 or int(summary[0][1])!=expected_failures or run.returncode!=(0 if label=='current' else 1):raise ValueError('Unexpected actual Attack outcome; retain '+str(case/'run.log'))
        for row in rows:
            raster=case/'host'/row['raster'];assert raster.stat().st_size==64*48*2
            row['rasterSha256']=sha(raster)
        result=dict(label=label,exit=run.returncode,states=48,failures=expected_failures,checks=int(summary[0][2]),sourceInputManifestSHA256=sha(case/'source-inputs.json'),instrumented={name:sha(frozen/name) for name in sorted(expected)},rows=rows)
        (case/'report.json').write_text(json.dumps(result,indent=2)+'\n');results.append(result)
    report=dict(schema='dnbox-native-attack-shape/1',thresholds={'minimumRisingWidthMaximumFraction':.1,'maximumRisingWidthMinimumFraction':.4,'glyphDesignViewboxWidth':32},historicalCommit=receipt['commit'],historicalGitBlob=blob,historicalBlockSha256=hashlib.sha256(historic.encode()).hexdigest(),results=results,limits='Six visible typed drum Attack controls/four current native scale families at settled min/max. Geometry from actual native polyline plus unchanged AA raster, not a whole-screen layout or live timing test. Historical Attack block only is composed into current source; not complete historical firmware recovery. Attack represents amount, not a time law. Check5 and full F2 remain unmet.')
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k!='results'}))
if __name__=='__main__':main()
