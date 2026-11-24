
from pathlib import Path
p=Path('firmware/src/app/engine.c')
s=p.read_text()
key="        int status = original_runtime_render(&original_bass[bv], bv ? &original_workspace_b : &original_workspace,"
assert s.count(key)==1
s=s.replace(key,"""        if (original_bass[bv].kind == ORIGINAL_SOUND_SYNTHONE) {
            extern void host_s1_clock_watch(unsigned, const float *, unsigned);
            host_s1_clock_watch((unsigned)bv, original_bass[bv].dsp.synthone.e.t, note_lock_applied[bv]);
        }
"""+key)
s+="""
void host_s1_clock_peek(unsigned owner, float target[6])
{
    static const unsigned ids[] = {15, 47, 36, 43, 139, 64};
    for (unsigned i = 0; i < 6; ++i) target[i] = original_bass[owner].dsp.synthone.e.t[ids[i]];
}
"""
p.write_text(s)

p=Path('firmware/src/app/s1c_part.c')
s=p.read_text()
key="#if X0X_S1_EXTRA\n    part->extra_live_mask = 0;"
assert s.count(key)==1
s=s.replace(key,"""    {
        extern void host_clock_witness_words(s1c *, unsigned);
        host_clock_witness_words(e, program);
    }
"""+key)
p.write_text(s)
