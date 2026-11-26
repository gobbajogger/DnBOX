#!/usr/bin/env python3
"""Add authored tuning to both renderers and keep exact original arp note boundaries."""
from pathlib import Path
import shutil,sys
here=Path(__file__).resolve().parent
d=Path(sys.argv[1])
shutil.copyfile(here/'authored-tuning/tuning-load.h',d/'tuning-load.h')
def patch(path, pairs):
    p=d/path;s=p.read_text()
    for a,b in pairs:
        assert s.count(a)==1, (path,a,s.count(a))
        s=s.replace(a,b)
    p.write_text(s)
patch('render.c',[
    ('static s1_dry_part part;', '''#include "tuning-load.h"
float s1_ab_authored_tuning[127];
int s1_ab_has_authored_tuning;
static s1_dry_part part;'''),
    ('if(argc!=5&&argc!=7)return 2;','if(argc!=5&&argc!=7)return 2;\n    s1_ab_has_authored_tuning=load_s1_authored_tuning(s1_ab_authored_tuning);')])
patch('s1c.c',[
    ('static float note_hz(const s1c *s, int n)\n{','extern float s1_ab_authored_tuning[127];\nextern int s1_ab_has_authored_tuning;\nstatic float note_hz(const s1c *s, int n)\n{'),
    ('    float ref = s->t[A4];','    if(s1_ab_has_authored_tuning && n<127)return s1_ab_authored_tuning[n];\n    float ref = s->t[A4];')])
patch('orig.cpp',[
    ('#include "stimulus.h"','#include "stimulus.h"\n#include "tuning-load.h"'),
    ('    CHECK(o.init(fixture.tables,S1AB_RATE,init,150,fixture.tuning,128));','''    float authored[128];
    std::memcpy(authored,fixture.tuning,sizeof authored);
    const bool custom=load_s1_authored_tuning(authored);
    CHECK(o.init(fixture.tables,S1AB_RATE,init,150,custom?authored:fixture.tuning,128));
    if(custom){
        const char *npo=std::getenv("S1_AUTHORED_TUNING_NPO");
        CHECK(npo);
        const int count=std::atoi(npo);CHECK(count>0&&count<=128);
        o.sequencer.setNotesPerOctave(count);
    }''')])
