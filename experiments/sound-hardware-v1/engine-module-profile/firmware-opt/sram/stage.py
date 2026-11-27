from pathlib import Path
import hashlib,sys
p=Path(sys.argv[1]);s=p.read_text()
print('donor',hashlib.sha256(p.read_bytes()).hexdigest())
point='static float voice_run_kr(s1c *s, s1c_voice *v)\n'
a=s.index(point)
start=s.index('    if (--v->kc <= 0) {\n        v->kc = S1C_KR;\n',a)
body=start+len('    if (--v->kc <= 0) {\n')
end=s.index('\n    }\n    int bl = P(BLE) > 0.0f;',body)
cold='static __attribute__((noinline)) void voice_kr_refresh(s1c *s, s1c_voice *v, int mono, int ft)\n{\n'
cold+='\n'.join(line[4:] if line.startswith('    ') else line for line in s[body:end].splitlines())+'\n}\n'
hot='''#if DNBOX_S1_VOICE_SRAM
#define S1_VOICE __attribute__((section(".ram_text.s1_voice"),noinline,used))
#else
#define S1_VOICE
#endif
static S1_VOICE float voice_run_kr(s1c *s, s1c_voice *v)
'''
s=s[:a]+cold+hot+s[a+len(point):start]+'    if (--v->kc <= 0) voice_kr_refresh(s,v,mono,ft);'+s[end+len('\n    }'):]
Path(sys.argv[2]).write_text(s)
