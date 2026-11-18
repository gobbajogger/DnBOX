#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Render current reached screens and check semantic captions in final pixels.

Complements component/glyph detectors; it does not assert complete R22 design,
all possible UI states, native memory fit, or the aggregate lane gate.
"""
import argparse,json,os,re,shutil,subprocess,sys
from pathlib import Path
from instrument import instrument
sys.path.append(str(Path(__file__).resolve().parents[1]/"ui_defects/first_slice"))
from screen_walk import validate as validate_frames
from full_geometry import validate_audits

def run(source,output):
    source=source.resolve();output=output.resolve()
    if output.exists():raise ValueError('use a new output directory; stale results are not accepted')
    if output.is_relative_to(source):raise ValueError('output must be outside source')
    output.mkdir(parents=True);work=output/'source'
    shutil.copytree(source,work,symlinks=True,ignore=shutil.ignore_patterns('.git','build','__pycache__','node_modules','.output'))
    instrument(work)
    env=dict(os.environ,SMC_UI_OUT=str(output/'host'),SMC_UI_TEST_FILE='tests/ui_geometry/capture.c')
    env.pop('GEOMETRY_MUTATE',None)
    env.pop('AUDIT_MUTATE_SKIP',None);env.pop('AUDIT_MUTATE_ZERO',None)
    for key in ('PIXEL_MUTATE_BLANK','PIXEL_MUTATE_SHIFT','PIXEL_MUTATE_OVERPAINT','ENVELOPE_MUTATE_CURVE','ENVELOPE_MUTATE_BLANK'):env.pop(key,None)
    for key in ('PRESENCE_MUTATE_VALUE','PRESENCE_MUTATE_GLYPH','PRESENCE_MUTATE_LIST','PRESENCE_MUTATE_SYNTH','PRESENCE_MUTATE_STRIP','PRESENCE_MUTATE_ENV_KNOB','PRESENCE_MUTATE_ENV_STAGE','PRESENCE_MUTATE_ENV_PERCENT','PRESENCE_MUTATE_ENV_MISSING'):env.pop(key,None)
    with (output/'render.log').open('w') as log:
        result=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=600)
    # Validate actual output before any PASS or mutants: catches truncated walks,
    # failed PNG writes (including disk full), blank frames and wrong identities.
    inventory=validate_frames((output/'render.log').read_text(),output/'host',result.returncode)
    (output/'inventory.json').write_text(json.dumps(inventory,indent=2)+'\n')
    audits=validate_audits((output/'render.log').read_text())
    matches=re.findall(r'^GEOMETRY findings=(\d+) screens=(\d+)$',(output/'render.log').read_text(),re.M)
    report={'status':'FAIL','rendererExit':result.returncode,'audits':audits,'scope':'semantic-caption clipping/occlusion in reached final frames; component/glyph geometry is separate'}
    if len(matches)==1:
        findings,screens=map(int,matches[0]);report.update(findings=findings,screens=screens)
        if result.returncode==0 and findings==0 and screens==inventory['screens'] and audits['findings']==findings:
            mutant_env=dict(env,GEOMETRY_MUTATE='1');mutant_env.pop('SMC_UI_SHOTS',None)
            with (output/'negative.log').open('w') as log:
                negative=subprocess.run([str(output/'host/test')],cwd=work,env=mutant_env,stdout=log,stderr=subprocess.STDOUT,timeout=600)
            validate_audits((output/'negative.log').read_text())
            m=re.findall(r'^GEOMETRY findings=(\d+) screens=(\d+)$',(output/'negative.log').read_text(),re.M)
            report['negativeExit']=negative.returncode
            if negative.returncode!=0 and len(m)==1 and int(m[0][0])>0 and int(m[0][1])==screens:report['status']='PASS'
    presence=re.findall(r'^PRESENCE checks=(\d+) missing=(\d+)$',(output/'render.log').read_text(),re.M)
    report['presenceScope']='active 909/808 controls, region-bound reachable lists, synth controls and ordinary knob strips; not all-screen R22 qualification'
    if len(presence)!=1 or int(presence[0][0])==0 or int(presence[0][1])!=0:report['status']='FAIL'
    else:
        report['presenceChecks']=int(presence[0][0])
        for key in ('PRESENCE_MUTATE_VALUE','PRESENCE_MUTATE_GLYPH','PRESENCE_MUTATE_LIST','PRESENCE_MUTATE_SYNTH','PRESENCE_MUTATE_STRIP','PRESENCE_MUTATE_ENV_KNOB','PRESENCE_MUTATE_ENV_STAGE','PRESENCE_MUTATE_ENV_PERCENT','PRESENCE_MUTATE_ENV_MISSING'):
            me=dict(env,**{key:'1'});me.pop('SMC_UI_SHOTS',None)
            logpath=output/(key.lower()+'.log')
            with logpath.open('w') as log:
                mrun=subprocess.run([str(output/'host/test')],cwd=work,env=me,stdout=log,stderr=subprocess.STDOUT,timeout=600)
            validate_audits(logpath.read_text())
            pm=re.findall(r'^PRESENCE checks=(\d+) missing=(\d+)$',logpath.read_text(),re.M)
            report[key]={'exit':mrun.returncode,'presence':pm}
            if mrun.returncode==0 or len(pm)!=1 or int(pm[0][1])==0:report['status']='FAIL'
    pix=re.findall(r'^GLYPH_PIXELS checks=(\d+) missing=(\d+) damaged=(\d+)$',(output/'render.log').read_text(),re.M)
    report['glyphPixelScope']='active 909/808 controls: positive raster deltas, own bank/cell bounds and final-frame survival'
    report['glyphPixels']=pix
    if len(pix)!=1 or int(pix[0][0])==0 or int(pix[0][1])+int(pix[0][2])!=0:report['status']='FAIL'
    else:
        for key in ('PIXEL_MUTATE_BLANK','PIXEL_MUTATE_SHIFT','PIXEL_MUTATE_OVERPAINT','ENVELOPE_MUTATE_CURVE','ENVELOPE_MUTATE_BLANK'):
            me=dict(env,**{key:'1'});me.pop('SMC_UI_SHOTS',None)
            lp=output/(key.lower()+'.log')
            with lp.open('w') as log:mrun=subprocess.run([str(output/'host/test')],cwd=work,env=me,stdout=log,stderr=subprocess.STDOUT,timeout=600)
            validate_audits(lp.read_text())
            pm=re.findall(r'^GLYPH_PIXELS checks=(\d+) missing=(\d+) damaged=(\d+)$',lp.read_text(),re.M)
            report[key]={'exit':mrun.returncode,'glyphPixels':pm}
            if mrun.returncode==0 or len(pm)!=1 or int(pm[0][1])+int(pm[0][2])==0:report['status']='FAIL'
            if key.startswith('ENVELOPE_'):
                expected={prefix+f'view-01-part-00-voice-{voice:02d}-page-01' for voice in (0,1) for prefix in ('','edge-0-','edge-1-')}
                actual=set(re.findall(r'^GLYPH_PIXEL_MISSING screen=(\S+)',lp.read_text(),re.M))
                report[key]['expectedGraphFailures']=sorted(expected)
                report[key]['actualGraphFailures']=sorted(actual)
                if actual!=expected:report['status']='FAIL'

    for key in ('AUDIT_MUTATE_SKIP','AUDIT_MUTATE_ZERO'):
        me=dict(env,**{key:'1'});me.pop('SMC_UI_SHOTS',None);lp=output/(key.lower()+'.log')
        with lp.open('w') as log:
            mrun=subprocess.run([str(output/'host/test')],cwd=work,env=me,stdout=log,stderr=subprocess.STDOUT,timeout=600)
        rejected=False
        try:validate_audits(lp.read_text())
        except ValueError:rejected=True
        gm=re.findall(r'^GEOMETRY findings=(\d+) screens=(\d+)$',lp.read_text(),re.M)
        report[key]={'exit':mrun.returncode,'auditRejected':rejected}
        if mrun.returncode!=0 or not rejected or len(gm)!=1 or int(gm[0][1])!=inventory['screens']:report['status']='FAIL'
    (output/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));return 0 if report['status']=='PASS' else 1

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    try:raise SystemExit(run(a.source,a.output))
    except (OSError,ValueError,subprocess.SubprocessError) as error:print(json.dumps({'status':'FAIL','reason':str(error)}));raise SystemExit(1)
