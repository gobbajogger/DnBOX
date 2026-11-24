from pathlib import Path
p=Path('firmware/src/app/engine.c')
s=p.read_text()
key="        bass_adapt_post(bv, bb[bv], bb[1-bv], (unsigned)n);"
assert s.count(key)==1
s=s.replace(key,key+"""
        {
            extern void host_lfo_capture(unsigned, const float *, const float *, unsigned);
            host_lfo_capture((unsigned)bv, bb[bv], bb[1-bv], (unsigned)n);
        }
""")
s+="""
void host_s1_wet_peek(unsigned owner, float enables[2], unsigned layout[5])
{
    const s1c *e = &original_bass[owner].dsp.synthone.e;
    enables[0] = e->t[S1C_P_DLON];
    enables[1] = e->t[S1C_P_RVON];
    layout[0] = (unsigned)e->pool_bytes;
    for (unsigned i = 0; i < 4; ++i) layout[i+1] = (unsigned)e->dl[i].len;
}
"""
p.write_text(s)
