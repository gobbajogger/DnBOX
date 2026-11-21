#!/usr/bin/env python3
"""Trace visible text on actual extended/edited frames; no ordinary walk/repack."""
import argparse,hashlib,json,os,re,shutil,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent

def once(s,a,b):
    if s.count(a)!=1:raise ValueError('nonunique visible-text hook '+a[:100])
    return s.replace(a,b,1)

def prepare(source,out):
    work=out/'source';shutil.copytree(source,work,symlinks=True,ignore=shutil.ignore_patterns('.git','build','__pycache__'))
    p=work/'firmware/src/gfx.c';s=p.read_text();marker='static void geo_blit(int x,int y) {\n if(!geo_enabled)return;'
    s=once(s,marker,(HERE/'visible-trace.inc').read_text()+'\n'+marker+'\n visible_commit(x,y);');p.write_text(s)
    p=work/'host/x0x_host.c';s=p.read_text();marker='static void lcd_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t c)\n{'
    s=once(s,marker,'static void visible_clear(int x,int y,int w,int h);\n'+marker+'\n visible_clear((int)x,(int)y,(int)w,(int)h);');p.write_text(s)
    p=work/'tests/host/smc_ui_test.c';s=p.read_text();s=once(s,' f2_extra_audit(name);',' visible_text_dump(name);\n f2_extra_audit(name);');p.write_text(s)
    # Keep the same layered F2 source and replace only its ordinary main with
    # the unchanged edit fixture main. Both invocations remain separate.
    p=work/'tests/ui_geometry/capture.c';s=p.read_text();marker='\nint main(void)\n{\n    unsigned total=0;'
    if s.count(marker)!=1:raise ValueError('nonunique ordinary capture main')
    prefix=s[:s.index(marker)]
    edit=(work/'tests/host/ui_step_lane_edit_test.c').read_text();marker='#include "smc_ui_test.c"';edit=edit[edit.index(marker)+len(marker):]
    edit=once(edit,'int main(void)\n{','int main(void)\n{\n f2_extra_enabled=1;f2_reset();')
    (work/'tests/ui_geometry/capture-visible-edited.c').write_text(prefix+'\n'+edit)
    return work

def flags(text,case):
    upper=text.upper();reasons=[]
    for pattern,label in [(r'PICKUP.*MOVE','pickup instruction'),(r'^PICK F\d','physical pickup instruction'),(r'\b(PRESS|TURN RIGHT|TEACH|REINSTALL|TRY AGAIN)\b','instructional wording'),(r'\b(HOLD|TAP|MOVE UP|MOVE DOWN)\b','action guidance'),(r'\?$','configuration question')]:
        if re.search(pattern,upper):reasons.append(label)
    if case.startswith('boot-') and any(s in upper for s in ('CRASH','STARTING','NO SOUND','USB IS ON','WEB INSTALLER','REINSTALL','TRY AGAIN')):reasons.append('boot safety/status obligation; retain for assessment')
    return reasons
