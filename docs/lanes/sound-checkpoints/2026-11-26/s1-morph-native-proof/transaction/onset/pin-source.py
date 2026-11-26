from pathlib import Path
import sys
p=Path(sys.argv[1])/'tree/firmware/src/app/engine.c';t=p.read_text()
inject = '''/* Native diagnostic only: no USB or wall-clock authority over these hits. */
volatile uint32_t d9_test_timeline[4];
static void d9_test_keys(void)
{
    const uint32_t f=d8_frame;
    if (f==65536u || f==69632u || f==73728u) {
        d9_test_timeline[0]=f;
        s_drum(0,0,DR_RD,1.0f);
        s_drum(0,0,DR_CH,0.75f);
        s_drum(0,0,DR_BD,1.0f);
        s_drum(0,1,D8_BD,1.0f);
    }
}
'''
a='void engine_render(int32_t *out_lr, uint32_t n)'
assert t.count(a)==1
t=t.replace(a,inject+a)
a='    while (done < n) {\n        uint32_t k = seq_until_event(&seq, n - done);'
assert t.count(a)==1
t=t.replace(a,'''    d9_test_keys();
    while (done < n) {
        uint32_t k = seq_until_event(&seq, n - done);
        if (k>128u) k=128u;''')
a='            mix_targets(PART_909, mix_level[PART_909], solo_parts);'
assert t.count(a)==1
t=t.replace('static uint8_t eng_808_split','extern volatile uint32_t d9_test_timeline[4];\nstatic uint8_t eng_808_split',1)
t=t.replace(a,'''            if (d8_frame==73728u) {
                d9_test_timeline[1]=*(const uint32_t *)(const void*)&dry_l[8];
                d9_test_timeline[2]=*(const uint32_t *)(const void*)&rev[8];
            }
'''+a)
a='    {   /* globals in registers: the stores below cannot reload them per sample */'
assert t.count(a)==1
t=t.replace(a,'''    if (d8_frame==73984u)
        d9_test_timeline[3]=(uint32_t)(int32_t)(out_l[8]*out_scale);
'''+a)
# Preserve the exact bit test while avoiding strict aliasing.
for n in (1,2):
 old=f'd9_test_timeline[{n}]=*(const uint32_t *)(const void*)&'
 channel='dry_l' if n==1 else 'rev'
 t=t.replace(old+channel+'[8];',f'{{ uint32_t bits; __builtin_memcpy(&bits,&{channel}[8],4); d9_test_timeline[{n}]=bits; }}')
p.write_text(t)
