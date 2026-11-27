from pathlib import Path
import json,re,subprocess,sys
d,t,a=map(Path,sys.argv[1:])
rows={}
for model in ('elite','pro'):
    p=d/model; elf=p/'never-install.elf'
    audit=json.loads(subprocess.check_output(['python',a,'synth-one','--elf',elf,
        '--link-map',p/'lab.map','--nm',t/'pi32v2/bin/nm']))
    def dump(op):return subprocess.check_output([t/'common/bin/objdump',op,elf],text=True)
    sections,syms=dump('-h'), dump('-t')
    def sec(k):return int(re.search(r'^\s*\d+\s+\.'+k+r'\s+([0-9a-f]+)',sections,re.M)[1],16)
    pairs=(('_rt_load','ram_text'),('_c1_load','c1_text'),('_data_load','data'))
    sizes={k:sec(k) for k in ('text','pool','bss','lab_slot3','lab_fittab','lab_fdata',*[k for _,k in pairs])}
    def load(k):return int(re.search(r'^([0-9a-f]+).* '+k+r'$',syms,re.M)[1],16)
    end=max(sizes['text'],*[load(sym)-0x02000120+sizes[k] for sym,k in pairs if sizes[k]])
    used=((end+3)&~3)+4
    rows[model]={'allocated_sections':sizes,'xip_used':used,'xip_free':0x86ee0-used,
       'pool_beyond_floor_and_margin':0x54000-sizes['pool']-8192-512, 'module_audit':audit}
print(json.dumps(rows,indent=2))
