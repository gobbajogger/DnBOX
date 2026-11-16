#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Measure actual renderer dispatch coverage without claiming screen completeness."""
import argparse,hashlib,json,re,shutil,subprocess
from pathlib import Path

FUNCTIONS=['draw_drum','draw_303','draw_drum_voice','draw_part_sound','draw_break',
           'draw_home','draw_song','draw_fx','draw_mix','draw_perf','draw_readout',
           'draw_list','draw_ask','draw_help']
BRANCHES={
 'padcal-body':'if (smk_padcal_on()) {',
 'clip-wait-body':'if (smk_clip_mode_on() && ui.overlay == O_NONE && !ui.help && smk_clip_wait()) {',
 'clip-grid-body':'if (smk_clip_mode_on() && ui.overlay == O_NONE && !ui.help) {',
 'smc-body':'if (smc_ui_active()) {',
 'project-transfer-header':'if (project_busy) {'}
ROUTES=FUNCTIONS+list(BRANCHES)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def instrument(ui,drum):
    if ui.count('#include "x0x.h"')!=1:raise ValueError("Expected one UI include anchor")
    ui=ui.replace('#include "x0x.h"','#include "x0x.h"\nstatic unsigned fwui_route_hits[%d];'%len(ROUTES),1)
    for i,name in enumerate(FUNCTIONS):
        pattern=r'(static UI_DRAW void '+name+r'\([^;{}]*\)\s*\{)'
        target=drum if name=='draw_drum_voice' else ui
        if len(re.findall(pattern,target))!=1:raise ValueError('Expected one definition: '+name)
        target=re.sub(pattern,lambda m:m[1]+'\n    fwui_route_hits[%d]++;'%i,target,count=1)
        if name=='draw_drum_voice':drum=target
        else:ui=target
    # Scope exceptional dispatch branches to actual entry functions, not similarly
    # spelled input/autosave branches elsewhere in this large translation unit.
    for name,anchor in BRANCHES.items():
        start=ui.index('void ui_frame(void)') if name in ('smc-body','project-transfer-header') else ui.index('static UI_DRAW void draw_main(void)')
        end=ui.find('\n}\n',start)+3
        segment=ui[start:end]
        if segment.count(anchor)!=1:raise ValueError('Expected one dispatch anchor: '+name)
        ui=ui[:start]+segment.replace(anchor,anchor+'\n        fwui_route_hits[%d]++;'%ROUTES.index(name),1)+ui[end:]
    return ui,drum

EXTRA=r'''
int main(void)
{
    CHECK(ordinary_inventory_main()==0);
    setup(V_HOME,PART_303A);held_btn=1u<<B_SEL;input();
    now_ms+=HELP_HOLD+1;input();CHECK(ui.help==B_SEL+1);ui_frame();
    held_btn=0;input();CHECK(!ui.help);
    setup(V_HOME,PART_303A);ask(ACT_CLEAR_PART,PART_303A,"CLEAR PART?","TEST ONLY");
    CHECK(ui.overlay==O_ASK);ui_frame();button_tap(B_HOME);CHECK(ui.overlay==O_NONE);
    setup(V_HOME,PART_909);fader(0,0);CHECK(smc_ui_active());ui_frame();
    smc_peek_cancel();ui_frame();
    setup(V_MIX,PART_909);ui.touched=0;ui.touch_until=now_ms+5000;ui_frame();
    /* Existing fixture clip body is deliberately NOT accepted as actual firmware. */
    setup(V_HOME,PART_909);clip_grid_mode=1;ui_frame();clip_grid_mode=0;
    setup(V_HOME,PART_909);clip_grid_mode=1;clip_waiting=1;ui_frame();
    clip_waiting=0;clip_grid_mode=0;
    setup(V_HOME,PART_909);project_busy=1;ui_frame();project_busy=0;
    for(unsigned i=0;i<sizeof fwui_route_hits/sizeof fwui_route_hits[0];i++)
        printf("ROUTE %u %u\n",i,fwui_route_hits[i]);
    return 0;
}
'''
def main():
    p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();source=a.source.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    frozen=out/'source';frozen.mkdir()
    for name in ('firmware','host','tests','tools','assets','experiments'):
        shutil.copytree(source/name,frozen/name,symlinks=True,ignore=shutil.ignore_patterns('__pycache__'))
    for name in ('build.sh',):shutil.copy2(source/name,frozen/name)
    inputs={str(p.relative_to(frozen)):sha(p) for p in frozen.rglob('*') if p.is_file()}
    (out/'source-inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    ui=frozen/'firmware/src/app/ui.c';drum=frozen/'firmware/src/app/ui_drum_voice.inc'
    u,d=instrument(ui.read_text(),drum.read_text());ui.write_text(u);drum.write_text(d)
    inventory=frozen/'tests/host/ui_screen_inventory.c';text=inventory.read_text()
    if text.count('int main(void)')!=1:raise ValueError('Inventory main changed')
    text=text.replace('int main(void)','static int ordinary_inventory_main(void)').replace('shot(name);','(void)name;')
    target=frozen/'tests/host/ui_route_coverage_test.c';target.write_text(text+EXTRA)
    expected={'firmware/src/app/ui.c','firmware/src/app/ui_drum_voice.inc'}
    changed={path for path,digest in inputs.items() if sha(frozen/path)!=digest}
    if changed!=expected:raise ValueError('Unexpected instrumentation mutations: '+str(changed))
    env=__import__('os').environ.copy();env.update(SMC_UI_TEST_FILE='tests/host/ui_route_coverage_test.c',SMC_UI_OUT=str(out/'host'))
    with (out/'first-run.log').open('w') as log:
        result=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=frozen,env=env,stdout=log,stderr=subprocess.STDOUT)
    records={int(i):int(n) for i,n in re.findall(r'^ROUTE (\d+) (\d+)$',(out/'first-run.log').read_text(),re.M)}
    if result.returncode or set(records)!=set(range(len(ROUTES))):raise ValueError('Renderer/coverage producer failed; retain first-run.log')
    rows=[{'id':name,'calls':records[i],'status':'OBSERVED' if records[i] else 'NOT_RUN'} for i,name in enumerate(ROUTES)]
    for row in rows:
        if row['id'] in ('clip-grid-body','padcal-body'):
            row['status']='NOT_RUN';row['reason']='Device renderer is substituted in this host fixture; route entry is not renderer evidence.'
    report={'schema':'dnbox-render-dispatch-coverage/1','coverageComplete':False,'routes':rows,
        'sourceInputManifestSHA256':sha(out/'source-inputs.json'),'sourceUI_SHA256':inputs['firmware/src/app/ui.c'],
        'instrumentedFiles':sorted(expected),'limits':'Function/branch entry counts only. No pixel, style, complete state inventory, hardware or lane qualification. No PNGs saved.'}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
if __name__=='__main__':main()
