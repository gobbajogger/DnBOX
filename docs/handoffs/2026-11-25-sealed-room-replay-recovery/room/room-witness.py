from pathlib import Path
import json,re,struct,subprocess,sys
elf,folder,offsets=map(Path,sys.argv[1:4])
report=subprocess.check_output(['readelf','-sW',elf],text=True)
arm=re.search(r'^\s*\d+:\s+([0-9a-fA-F]+)\s+(\d+) OBJECT .*\boriginal_bass$',report,re.M)
assert arm
state=(folder/'ram.bin').read_bytes()
span,fb,mem,dec,dlon,rvon,rvmix=map(int,re.findall(r'^\s*\.long\s+(\d+)',offsets.read_text(),re.M))
assert span*2==int(arm[2])
base=int(arm[1],16)-0x01c00000
assert 0<=base<=len(state)-span*2
def at(spec,o):return struct.unpack_from('<'+spec,state,base+o)[0]
new=bool(re.search(r'\bFUNC\s+LOCAL\s+DEFAULT\s+\d+ rv8_step$',report,re.M))
r={'rv8_present':new,'profile':sys.argv[4],'room_fb':at('f',fb),'room_memory':hex(at('I',mem)),
   'delay_decimation':at('I',dec),'delay_on':at('f',dlon),'reverb_on':at('f',rvon),'reverb_mix':at('f',rvmix)}
print(json.dumps(r),flush=True)
if sys.argv[4]=='active':
    assert at('I',mem)!=0 and at('I',dec)!=0 and r['delay_on']==0 and r['reverb_on']==1 and r['reverb_mix']>0
    assert (r['room_fb']==-2.0)==new
