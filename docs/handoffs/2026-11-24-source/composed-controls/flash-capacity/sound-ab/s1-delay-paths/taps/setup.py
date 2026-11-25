import sys
from pathlib import Path
d=Path(__file__).parent
s=(d.parents[1]/'ab-latest-headless/firmware/src/dsp/s1c/s1c.c').read_text()
use=int(sys.argv[1])
# Every test preserves all paths except for explicitly muting one of the two
# inputs to the right output line, for both same-budget and full-rate models.
if use==1: s=s.replace('iR + r, T, fbk','iR, T, fbk').replace('dR + oR, T, fbk','dR, T, fbk')
if use==2: s=s.replace('iR + r, T, fbk','r, T, fbk').replace('dR + oR, T, fbk','oR, T, fbk')
a=s.index('        float dm = P(DLMIX) * P(DLON);')
b=s.index('        /* reverb:',a)
s=s[:a]+'''        outl[f]=oL;outr[f]=oRR;continue;
        }
'''+s[b:]
(d/'firmware/src/dsp/s1c/s1c.c').write_text(s)
driver=(d/'driver.pre').read_text().replace('if(!s1_part_program(&part,index))return 3;','''if(!s1_part_program(&part,index))return 3;
    static uint16_t oracle_pool[120000];
    if(getenv("FULL_RATE")) {part.e.pool=(uint8_t*)oracle_pool;part.e.pool_bytes=sizeof oracle_pool;}''')
(d/'render.c').write_text(driver)
