#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Retain chord lifecycle/assertions, replace only its graphics stubs with production graphics."""
from pathlib import Path
import hashlib
import json
import os
import sys

out = Path(sys.argv[1])
source = Path('tests/host/smk_clip_chord_display_test.c')
s = source.read_text()
def unique(text, anchor):
    if text.count(anchor) != 1:
        raise ValueError('expected one extraction anchor: ' + anchor)
    return text.index(anchor)
start = unique(s, 'static char chord_draw')
end = unique(s, 'static void text_c')
main = unique(s, 'static void fresh(void)')
assert start < end < main
hook = '#include <assert.h>\n' + s[start:end]
assert hook.count('static int32_t cv_text(') == 1
assert hook.count('return x+tw(f,s);') == 1
hook = hook.replace('static int32_t cv_text(', 'static int32_t observed_cv_text(')
hook = hook.replace('return x+tw(f,s);', '''/* Observe actual opaque font ink, then call the unchanged production renderer. */
    for (const unsigned char *p=(const unsigned char*)s; *p; p++) {
        unsigned ch=*p; if(ch<f->first || ch>f->last) ch='?';
        unsigned gi=ch-f->first,w=font_bitmap_width(f,gi),stride=(w+1)/2;
        const uint8_t *data=f->data+font_offset(f,gi);
        for(unsigned yy=0;yy<f->h;yy++) for(unsigned xx=0;xx<w;xx++) {
            unsigned a=data[yy*stride+xx/2]; a=(xx&1)?a&15:a>>4;
            if(a) { assert(pen-(int)f->pad+(int)xx>=0); assert(pen-(int)f->pad+(int)xx<(int)cv_w); assert(y+(int)yy>=0 && y+(int)yy<(int)cv_h); }
        }
        pen+=font_advance(f,gi);
    }
    return cv_text(x,y,f,s,c);''')
hook = hook.replace('(void)c;', 'int32_t pen=x;')
hook += '\n#define cv_text observed_cv_text\n'
host = out / 'actual-clip-key-host.inc'
text = host.read_text()
anchor = '#include "actual-clip-key-renderers.inc"'
unique(text, anchor)
helper_anchor = 'static void text_c('
unique(text, helper_anchor)
host.write_text(text.replace(helper_anchor, hook + '\n' + helper_anchor))
(out / 'chord-real.c').write_text('#include <assert.h>\n#include "actual-clip-key-host.inc"\n' + s[main:])
inputs = [source, Path(__file__), Path('tests/host/ui_clip_chord_graphics_test.sh'), Path('tests/host/ui_clip_strip_domains_test.c')]
receipt = {'schema': 'dnbox-actual-chord-graphics/1', 'inputs': {}, 'generated': {}}
for p in inputs:
    receipt['inputs'][str(p).replace(str(Path.cwd())+'/', '')] = hashlib.sha256(p.read_bytes()).hexdigest()
for p in sorted(out.glob('*.inc')) + [out/'felucca_font.h', out/'chord-real.c']:
    receipt['generated'][p.name] = hashlib.sha256(p.read_bytes()).hexdigest()
receipt['setupProvenanceSha256'] = hashlib.sha256((out/'provenance.json').read_bytes()).hexdigest()
receipt['compileCommand'] = [os.environ.get('CC', 'cc'), '-DSMK_MODEL="SMK-37 Pro"', '-std=gnu99', '-O1', '-Ifirmware/src', '-Ifirmware/src/app', '-Ifirmware/hal', '-I$OUT', '$OUT/chord-real.c', 'firmware/src/app/smk_clip_arp_share.c', '-lm', '-o', '$OUT/chord-real']
receipt['stripCompileCommand'] = [os.environ.get('CC', 'cc'), '-DSMK_MODEL="SMK-37 Pro"', '-std=gnu99', '-O1', '-Ifirmware/src', '-Ifirmware/src/app', '-Ifirmware/hal', '-I$OUT', 'tests/host/ui_clip_strip_domains_test.c', 'firmware/src/app/smk_clip_arp_share.c', '-lm', '-o', '$OUT/strip-domains']
receipt['scope'] = 'Original lifecycle/test assertions preserved; production drawing replaces no-op graphics. Host MIDI/USB mocks remain. Not device or visual review.'
(out/'chord-graphics-provenance.json').write_text(json.dumps(receipt,indent=2)+'\n')
