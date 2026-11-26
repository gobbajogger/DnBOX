from pathlib import Path
import difflib,sys
root=Path(sys.argv[1]); diff=[]
def edit(f,pairs):
 a=(root/f).read_text(); b=a
 for x,y in pairs:
  assert b.count(x)==1,(f,x)
  b=b.replace(x,y)
 diff.extend(difflib.unified_diff(a.splitlines(True),b.splitlines(True),fromfile='a/'+f,tofile='b/'+f))
edit('firmware/src/app/engine.c',[
 ('extern volatile uint32_t d9_test_timeline[4];','''extern volatile uint32_t d9_test_timeline[4];
#if DNBOX_LAB_FIRST_CUE_REUSE
#if !X0X_COLD_COMPANION_LAB || !defined(X0X_CORE_PLAN) || X0X_CORE_PLAN != 4
#error first-cue reuse is diagnostic and requires the fixed warm 909/808 Core0 plan
#endif
#if DNBOX_LAB_MISSED_BUSY || DNBOX_LAB_FIRST_CUE
#error do not combine first-cue word reuse with other attribution recorders
#endif
#endif'''),
('''            if (d8_frame==73728u) {
                { uint32_t bits; __builtin_memcpy(&bits,&dry_l[8],4); d9_test_timeline[1]=bits; }
                { uint32_t bits; __builtin_memcpy(&bits,&rev[8],4); d9_test_timeline[2]=bits; }
            }''','''#if DNBOX_LAB_FIRST_CUE_REUSE
            if (!core && d8_frame==65536u && n>8u && d9_test_timeline[0]==0xa0000000u) {
                uint32_t bits; __builtin_memcpy(&bits,&dry_l[8],4);
                d9_test_timeline[1]=bits;
                d9_test_timeline[0]=0xa0000001u;
            }
#else
            if (d8_frame==73728u) {
                { uint32_t bits; __builtin_memcpy(&bits,&dry_l[8],4); d9_test_timeline[1]=bits; }
                { uint32_t bits; __builtin_memcpy(&bits,&rev[8],4); d9_test_timeline[2]=bits; }
            }
#endif'''),
('''                bfx_shape(PART_808, left808, right808, t1, t2, 4, (int)n);
            } else {
                for (i = 0; i < n; i++)
                    left808[i] = t1[i] = t2[i] = right808[i] = 0.0f;
                drum808_render_st_masked(&d808, left808, right808, t1, t2, (int)n, (uint16_t)(arg >> 9 & 0x7FFu));
                bfx_shape(PART_808, left808, right808, t1, t2, 4, (int)n);
            }''', '''                bfx_shape(PART_808, left808, right808, t1, t2, 4, (int)n);
            } else {
                for (i = 0; i < n; i++)
                    left808[i] = t1[i] = t2[i] = right808[i] = 0.0f;
                drum808_render_st_masked(&d808, left808, right808, t1, t2, (int)n, (uint16_t)(arg >> 9 & 0x7FFu));
                bfx_shape(PART_808, left808, right808, t1, t2, 4, (int)n);
            }
#if DNBOX_LAB_FIRST_CUE_REUSE
            if (!core && d8_frame==65536u && n>8u && d9_test_timeline[0]==0xa0000001u) {
                uint32_t bits; __builtin_memcpy(&bits,&left808[8],4);
                d9_test_timeline[2]=bits;
                d9_test_timeline[0]=0xa0000003u;
            }
#endif'''),
('        d9_test_timeline[0]=f;', '''#if DNBOX_LAB_FIRST_CUE_REUSE
        if (f==65536u) d9_test_timeline[0]=0xa0000000u;
#else
        d9_test_timeline[0]=f;
#endif'''),
('''    if (d8_frame==73984u)
        d9_test_timeline[3]=(uint32_t)(int32_t)(out_l[8]*out_scale);''','''#if !DNBOX_LAB_FIRST_CUE_REUSE
    if (d8_frame==73984u)
        d9_test_timeline[3]=(uint32_t)(int32_t)(out_l[8]*out_scale);
#endif'''),
])
edit('firmware/src/app/main_fm1.c',[
 ('static uint8_t safe_mode;', '''static uint8_t safe_mode;
#if DNBOX_LAB_FIRST_CUE_REUSE
extern volatile uint32_t d9_test_timeline[4];
_Static_assert(REND_FRAMES==256u && HALF_FRAMES==512u,"first cue marker size");
#endif'''),
('''            uac_out_mix(o + 2u * k, REND_FRAMES);   /* USB audio from the host, after the tap and MASTER */
#endif''','''            uac_out_mix(o + 2u * k, REND_FRAMES);   /* USB audio from the host, after the tap and MASTER */
#endif
#if DNBOX_LAB_FIRST_CUE_REUSE
            if (d9_test_timeline[0]==0xa0000003u) {
                d9_test_timeline[3]=(uint32_t)o[2u*k+16u];
                d9_test_timeline[0]=0xb0000003u | ((k>>8)<<2);
            }
#endif'''),
('''        if (fm1_audio_free_half() != half) {''','''#if DNBOX_LAB_FIRST_CUE_REUSE
        const uint32_t exit_half=fm1_audio_free_half();  /* original endpoint read */
        const uint32_t cue=d9_test_timeline[0];
        if ((cue&0xfffffffbu)==0xb0000003u) {
            const uint32_t o=x0x_dbg.halves;
            d9_test_timeline[0]=o && o<=65536u ?
                0xc0000003u | (cue&4u) | (half<<8) | (exit_half<<9) | ((o-1u)<<11) :
                0xffffffffu;
        }
        if (exit_half != half) {
#else
        if (fm1_audio_free_half() != half) {
#endif'''),
])
sys.stdout.writelines(diff)
