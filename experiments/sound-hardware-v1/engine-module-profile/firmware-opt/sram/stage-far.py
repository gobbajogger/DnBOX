from pathlib import Path
import re,sys
s=Path(sys.argv[1]).read_text()
pt='static S1_VOICE float voice_run_kr(s1c *s, s1c_voice *v)\n'
a=s.index(pt); b=s.index('\n#define voice_run voice_run_kr\n',a)
names=('lfo10','clampf','env_step','voice_kr_refresh','morph','sine_t','ph_inc','ladder_a','ladder','tanf','hp2','cosf')
# Ports are immutable. Volatile lvalue forces vendor reality: long-range
# call through an actual absolute address, not a shortened relative call.
pre='#if DNBOX_S1_VOICE_FAR\n'
for n in names:pre+=f'static __typeof__(&{n}) const volatile s1_far_{n}=&{n};\n'
pre+='#else\n'
for n in names:pre+=f'#define s1_far_{n} {n}\n'
pre+='#endif\n'
body=s[a:b]
for n in names:body=re.sub(r'\b'+n+r'\(', 's1_far_'+n+'(',body)
post='''\n#if DNBOX_S1_VOICE_FAR
static __typeof__(&voice_run_kr) const volatile s1_voice_far=&voice_run_kr;
#define voice_run s1_voice_far
#else
#define voice_run voice_run_kr
#endif'''
s=s[:a]+pre+body+post+s[b+len('\n#define voice_run voice_run_kr'):]
Path(sys.argv[2]).write_text(s)
