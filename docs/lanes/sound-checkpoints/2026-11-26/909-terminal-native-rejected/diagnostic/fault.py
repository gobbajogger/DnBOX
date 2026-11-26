"""Inspect exact scratch RAM after emulator stops; no claim about continuous telemetry."""
import json, re, struct,sys
from pathlib import Path
D=Path(__file__).resolve().parent; R=D.parent.parent
sys.path.insert(0,str(R/'restored-harness-inspect/helpers'))
from guarded_replay import ELF_RAM,symbols_and_sections,ensure
elf,shot,mode,verb=sys.argv[1:]
_,sym,sections=symbols_and_sections(Path(elf))
names=['eng_terminal_claim','eng_terminal_fault','eng_stage','eng_808_body']
ensure(all(k in sym for k in names[:2])==(mode=='on'),'claim/fault symbols require candidate only')
for k in names[:2] if mode=='on' else []:
 a=sym[k]; addr,n,_=sections['.bss']
 ensure(a%4==0 and addr<=a<=addr+n-4 and a<sym['_pool_start'],k)
if verb=='preflight':
 print(json.dumps({'mode':mode,'claim_fields':{x:hex(sym[x]) for x in names if x in sym}}));sys.exit(0)
p=Path(shot); info={'mode':mode, 'whole_NOR_unchanged':None,'claim':None,'partial_fault':None}
ram=p/'ram.bin'
if ram.exists():
 b=ram.read_bytes()
 def word(n):
  o=sym[n]-ELF_RAM;ensure(0<=o<=len(b)-4,n);return struct.unpack_from('<I',b,o)[0]
 if mode=='on':
  info['claim']=word(names[0]); info['partial_fault']=word(names[1])
  info['recent_terminal_token']=word('eng_terminal_token')
  info['recent_worker_mask']=b[sym['eng_c1_mask']-ELF_RAM]
  info['recent_mix_stage']=word('eng_stage')
  ensure(info['claim'] in (0,1,2),'invalid final claim')

  info['partial_fault_meaning']={0:'no latched partial owner failure in final RAM',1:'C1 claimed 808 prefix before mutation but stopped before body publication',2:'C1 claimed bass A but stopped before stage-3 publication'}.get(info['partial_fault'],'invalid')
log=(p/'run.log').read_text(errors='replace')
info['no_erase_program_log']=re.findall(r'^flash: (\d+) erases / programs\s*$',log,re.M)==['0']
info['fault_lines']=[x for x in log.splitlines() if re.match(r'^\s*(?:EXCEPTION|TRAP |PANIC|HALT|illegal opcode|fatal:)',x,re.I)]
print(json.dumps(info))
ensure(not info['fault_lines'] and (mode=='off' or info['partial_fault']==0),'partial-owner trap/fault or undecodable dump')
ensure(info['no_erase_program_log'],'missing no-write trace')
