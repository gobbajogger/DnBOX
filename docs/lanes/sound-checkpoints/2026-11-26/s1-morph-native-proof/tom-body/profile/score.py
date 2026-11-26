import sys,json,subprocess
from collections import Counter
from pathlib import Path
elf,file=sys.argv[1:]
s={}
for line in subprocess.check_output(['readelf','-sW',elf],text=True).splitlines():
 c=line.split()
 if len(c)>=8 and c[6]!='UND' and c[3]=='FUNC' and c[2].isdigit():
  lo=int(c[1],16)
  s[c[7]]=(lo,lo+int(c[2]))
assert s['d9_render'][0]==0x207457a and s['d9_render'][1]==0x2076200
# Verified identical in disassembly for exact paired ELFs.
lo,samp,hi=0x207457a,0x2075c28,0x2076200
def lst(p,start,end):
 return sum(v for a,v in p.items() if start<=a<end)
c={k:Counter() for k in ['0','1','m0','m1']}
for line in Path(file).read_text().splitlines():
 core,p,v=line.split();c[core][int(p,16)]+=int(v)
p=c['0']
print(json.dumps({'window_core_weight':{k:sum(v.values()) for k,v in c.items()},
  'core0_d9_analog_and_noise_inlined':lst(p,lo,samp),'core0_d9_sampled_four_voices_inlined':lst(p,samp,hi),
  'analog_inlined_PC_windows':{k:lst(p,a,b) for k,a,b in [('init_noise',lo,0x2074642),('BD',0x2074642,0x2074b26),('SD',0x2074b26,0x2075030),('tom_group',0x2075030,0x2075776),('rim_and_clap_dispatch',0x2075776,samp)]},
  'core0_d9_named_helpers':{k:lst(p,*s[k]) for k in ('d9_emit_block','d9_render_clap','d9_render_st_masked','d9_bus','d9_shape_other','d9_warm') if k in s},
  'caveat':'Native PC weights in a 17ms wall window, not cycles of the single worst audio half. Inlined sampled voice loop shares one body per CH/OH/CR/RD; cannot split individual voices by PC. Shared helpers/flash cache misses not attributed to callers.'},indent=2))
