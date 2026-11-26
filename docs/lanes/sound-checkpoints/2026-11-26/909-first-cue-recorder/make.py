from pathlib import Path
import difflib,sys
root=Path(sys.argv[1])
all=[]
def edit(f, pairs):
 p=root/f; a=p.read_text();b=a
 for x,y in pairs:
  assert b.count(x)==1,(f,x)
  b=b.replace(x,y)
 all.extend(difflib.unified_diff(a.splitlines(True),b.splitlines(True),fromfile='a/'+f,tofile='b/'+f))
edit('firmware/src/app/main_fm1.c',[
('static uint8_t safe_mode;', '''static uint8_t safe_mode;
#if DNBOX_LAB_FIRST_CUE
#if !X0X_COLD_COMPANION_LAB
#error first-cue recorder is diagnostic only
#endif
/* 12 words: stage, actual sub-render start/range, 909 bus samples,
 * callback ordinal, ALNK flags, total elapsed us, four final DMA words. */
volatile uint32_t lab_first_cue[12] X0X_POOL __attribute__((aligned(4)));
#endif'''),
('''        if (fm1_audio_free_half() != half) {''','''#if DNBOX_LAB_FIRST_CUE
        const uint32_t exit_half=fm1_audio_free_half();  /* the original test read */
        if (lab_first_cue[0]==2) {
            const uint32_t off=lab_first_cue[2]>>16;
            if (off < HALF_FRAMES && HALF_FRAMES-off>8u) {
                const int32_t *first=abuf+half*HALF_WORDS+off*2u;
                lab_first_cue[5]=x0x_dbg.halves-1u;
                lab_first_cue[6]=(uint32_t)p | (half<<8) | (exit_half<<9) |
                    ((exit_half!=half)<<10);
                lab_first_cue[7]=x0x_dbg.last_us;
                lab_first_cue[8]=(uint32_t)first[0];
                lab_first_cue[9]=(uint32_t)first[1];
                lab_first_cue[10]=(uint32_t)first[16];
                lab_first_cue[11]=(uint32_t)first[17];
                lab_first_cue[0]=3;        /* complete, publish last */
            } else lab_first_cue[0]=0xffffffffu;
        }
        if (exit_half != half) {
#else
        if (fm1_audio_free_half() != half) {
#endif'''),
('''            engine_volume(o + 2u * k, REND_FRAMES);''','''#if DNBOX_LAB_FIRST_CUE
            if (lab_first_cue[0]==1 && lab_first_cue[1]==65536u) {
                lab_first_cue[2]|=k<<16;
                lab_first_cue[0]=2;       /* pin subcall within half, before further rendering */
            }
#endif
            engine_volume(o + 2u * k, REND_FRAMES);'''),
])
edit('firmware/src/app/engine.c',[
('extern volatile uint32_t d9_test_timeline[4];','''extern volatile uint32_t d9_test_timeline[4];
#if DNBOX_LAB_FIRST_CUE
extern volatile uint32_t lab_first_cue[12];
#endif'''),
('''            if (d8_frame==73728u) {''','''#if DNBOX_LAB_FIRST_CUE
            if (lab_first_cue[0]==1 && d8_frame==65536u) {
                uint32_t u;
                __builtin_memcpy(&u,&dry_l[0],4); lab_first_cue[3]=u;
                if (n>8u) { __builtin_memcpy(&u,&dry_l[8],4); lab_first_cue[4]=u; }
                lab_first_cue[2]|=0x200u | (n>8u ? 0x400u : 0u);
            }
#endif
            if (d8_frame==73728u) {'''),
('''    d9_test_keys();
    while (done < n)''','''#if DNBOX_LAB_FIRST_CUE
    if (d8_frame==65536u && lab_first_cue[0]==0) {
        lab_first_cue[1]=d8_frame;
        /* n is 256 maximum; low 9 bits hold n, flags start at bit 9. */
        lab_first_cue[2]=n;
        lab_first_cue[0]=1;
    }
#endif
    d9_test_keys();
    while (done < n)'''),
])
sys.stdout.writelines(all)
