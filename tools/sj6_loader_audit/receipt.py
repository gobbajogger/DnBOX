"""Read-only bounded static original-loader evidence. Never executes original code."""
from static import *

def u32(a):return struct.unpack('<I',data(a,4))[0]
def u64(a):return struct.unpack('<Q',data(a,8))[0]
def rel(a):return a+struct.unpack('<i',data(a,4))[0]
def check(cond,msg):
 if not cond:raise AssertionError(msg)
checks=[]
def claim(cond,msg):check(cond,msg);checks.append(msg)
# Decode import table from original Mach-O, not from retained derived JSON.
at=32
for _ in range(struct.unpack_from('<I',b,16)[0]):
 cmd,n=struct.unpack_from('<II',b,at)
 if cmd==0x80000034:fix,size=struct.unpack_from('<II',b,at+8)
 at+=n
version,starts_off,imports_off,symbols_off,count,fmt,sfmt=struct.unpack_from('<7I',b,fix)
claim((version,fmt,sfmt)==(0,1,0),'Original chained imports use format1/uncompressed symbols')
def binding(slot):
 val=u64(slot);check(val>>63==1,'expected chained bind')
 ordinal=val&0xffffff;check(ordinal<count,'import ordinal bound')
 record=struct.unpack_from('<I',b,fix+imports_off+ordinal*4)[0]
 nameoff=record>>9;pos=fix+symbols_off+nameoff
 return b[pos:b.index(0,pos)].decode()
append=binding(0x100732820);compare=binding(0x100732f08)
claim(append=='_$sSS6appendyySSF','0x1005d7cd4 -> GOT0x100732820 -> Swift String.append')
claim(compare=='_$ss27_stringCompareWithSmolCheck__9expectingSbs11_StringGutsV_ADs01_G16ComparisonResultOtF','0x1005d8724 -> GOT0x100732f08 -> Swift stringCompareWithSmolCheck')
# Struct descriptor and metadata accessor directly identify the value offset table.
preset_desc=0x10069da24;preset_meta=0x100711eb8
claim(rel(preset_desc+12)==0x1002c7398,'InstrumentPreset descriptor accessor0x1002c7398')
claim(u32(preset_desc+20)==27 and u32(preset_desc+24)==2,'InstrumentPreset27 fields, field-vector offset2 words')
claim(u32(preset_meta+16+14*4)==0x98,'InstrumentPreset ordinal14 parameterValues static offset0x98')
claim(u64(0x1007b7f80)==8 and u64(0x1007b7f88)==24,'ParameterValuePair identifier/value static slots8/24')
# BaseParameter raw value17 suffix/default dispatch tables.
suffix_branch=0x10001dc1c+4*struct.unpack('<H',data(0x100604fe0+17*2,2))[0]
default_branch=0x10001f2d4+4*data(0x100605194+17,1)[0]
claim(suffix_branch==0x10001f0ec and data(0x10061e9f0,17)==b'compressorAmount\0','ID17 suffix branch0x10001f0ec constructs compressorAmount')
claim(default_branch==0x10001f2e0,'ID17 default branch0x10001f2e0 RET after initial FMOV s0,wzr')
claim(u64(0x1007cfd80+8*14)==0x40 and u64(0x1007cfd80+8*15)==0x50,'InstrumentParameter static prefix/defaultOverride offsets0x40/0x50')
claim(list(data(0x1007bd730,3))==[212,213,214],'Loop skipped IDs212,213,214 do not include17')
# Exact instruction checks establish individual matching and fallback edges.
windows={
 'prefix_append':(0x100269840,0x40),
 'match_or_continue':(0x100269880,0x5c),
 'match_value_and_fallback':(0x100269abc,0x10c),
 'default_helper':(0x10001f2a8,0x3c),
 'id17_suffix':(0x10001f0ec,0x2c),
 'main_entry':(0x10026926c,0x190),
 'caller1':(0x1002671dc,0x60),
 'caller2_call':(0x100267c24,0x3c),
 'reset_before_load':(0x1002691c8,0xa4),
 'dictionary_parameter_pair':(0x1002c9f28,0x270),
 'dictionary_output':(0x1002ca1a0,0x1c4),
 'appstate_dictionary_call':(0x1000c0988,0x108),
 'appstate_to_loader':(0x1000c0a90,0x2ac),
 'caller2_prefix':(0x10026796c,0x2e4),
}
assertions=[(0x1005d7cd4,'adrp','x16, #0x100732000'),(0x1005d7cd8,'ldr','x16, [x16, #0x820]'),(0x1005d7cdc,'br','x16'),(0x1005d8724,'adrp','x16, #0x100732000'),(0x1005d8728,'ldr','x16, [x16, #0xf08]'),(0x1005d872c,'br','x16'),(0x10001f2ac,'fmov','s0, wzr'),(0x10001f2e0,'ret',''),(0x10026984c,'bl','#0x10001dbdc'),(0x100269874,'bl','#0x1005d7cd4'),(0x10026989c,'mov','w4, #0'),(0x1002698a0,'bl','#0x1005d8724'),(0x1002698b8,'tbnz','w21, #0, #0x100269acc'),(0x1002698d8,'b','#0x100269b44'),(0x100269b48,'bl','#0x10001f2a8'),(0x100269bfc,'bl','#0x10001f2a8'),(0x10026983c,'ldp','x21, x27, [x8]'),(0x100269ad8,'ldr','s9, [x25, x8]'),(0x100269b14,'ldr','s9, [x24, x8]')]
assertions += [(0x1000c0a8c,'bl','#0x1002c9360'),(0x1002ca0c0,'stp','x28, x22, [x8]'),(0x1002ca0d0,'str','s9, [x0, x8]'),(0x1002ca214,'str','x25, [x19, #0x330]'),(0x1000c0d20,'add','x1, x19, #0x430'),(0x1000c0d38,'bl','#0x10026796c'),(0x1002679a8,'stp','x21, x1, [x19, #0x140]'),(0x100267c40,'ldr','x24, [x19, #0x148]'),(0x100267c4c,'bl','#0x10026926c')]
for a,m,ops in assertions:
 ins=next(cs.disasm(data(a,4),a));claim((ins.mnemonic,ins.op_str)==(m,ops),f'{a:#x} {m} {ops}')
for name,(a,n) in windows.items():(ROOT/(name+'.asm')).write_text(dis(a,n)+'\n')
(ROOT/'loader-full.asm').write_text(dis(0x10026926c,0x10026a800-0x10026926c)+'\n')
# Mutated synthetic conditions must fail admission. No retained input is changed.
negative=[]
for name,cond in [('wrong_preset_offset',0x90==0x98),('wrong_default_target',0x10001f2dc==0x10001f2e0),('wrong_suffix_target',0x10001f0f8==0x10001f0ec),('wrong_import',compare==append)]:
 try:check(cond,name)
 except AssertionError:negative.append(name)
 else:raise AssertionError('negative accepted')
out={'scope':__doc__,'binary_sha256':sha,'disassembler':dependency_receipt,'checks':checks,'synthetic_rejection_checks':negative,'imports':{'append':append,'compare_expecting_zero_equality':compare},'static_layout':{'InstrumentPreset_parameterValues':0x98,'ParameterValuePair_identifier':8,'ParameterValuePair_value':24},'routine':'0x10026926c..0x10026a800','bounded_finding':'For a control with raw ID17 and current neon prefix, this routine searches current serialized neoncompressorAmount using exact Swift String equality, falls back to BaseParameter default0 if absent. It does not match the supplied legacy names on the inspected path. This is conditional on the records arriving here unmodified, not a whole-app migration result.','migration':'NOT_PROVEN','S5':'FAIL','missing_proof':['Full bank decode/library adaptation to this routine input, including alternate/indirect loaders and any preprocessing','Original-capable execution/source evidence connecting each legacy key and value through any migration to final control law'],'output_sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest()for p in [ROOT/(name+'.asm') for name in list(windows)+['loader-full']]}}
(ROOT/'receipt.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out,indent=2))
