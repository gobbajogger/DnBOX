"""Exception-scoped exact-source hot path, only removes 909 duplicate verification."""
from pathlib import Path
import sys,hashlib
p=Path(sys.argv[1]); original=p.read_bytes()
assert hashlib.sha256(original).hexdigest()=='d477d8c8f4d68e4fa1369b625a3680cf3887295dbb2752a82f5c40aa4da17be3'
s=original.decode()
def swap(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
swap('''static uint8_t eng_808_split, eng_d9_voice;
static d9_smp_t *eng_d9_seed;
static d9_future_t *volatile eng_d9_ready;''','static uint8_t eng_808_split;')
swap('''drum909_render_st_future(&d909,dry_l,dry_r,rev,dly,(int)n,
                        smc_audible_drums(solo,PART_909),eng_d9_voice,&eng_d9_ready);''','''drum909_render_st_cleared(&d909,dry_l,dry_r,rev,dly,(int)n,
                        smc_audible_drums(solo,PART_909));''')
a=s.index('    d9_future_t future;\n')
b=s.index('    /* Real platform:',a)
s=s[:a]+s[b:]
swap('    (void)future;\n','')
a=s.index('    eng_d9_seed=0;\n')
b=s.index('    eng_sync();\n',a)
s=s[:a]+s[b:]
swap('        eng_d9_seed = 0;\n','')
assert 'eng_d9_' not in s
p.write_text(s)
