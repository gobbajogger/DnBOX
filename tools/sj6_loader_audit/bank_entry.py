"""Bounded default bundled-bank input path, not final preset/control execution."""
from static import *
import argparse,io,zipfile,plistlib
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--ipa',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
checks=[]
def claim(ok,description):
 if not ok:raise AssertionError(description)
 checks.append(description)
def instruction(address,mnemonic,operands):
 got=list(cs.disasm(data(address,4),address));claim(len(got)==1 and (got[0].mnemonic,got[0].op_str)==(mnemonic,operands),hex(address)+' '+mnemonic+' '+operands)
def u64(a):return struct.unpack('<Q',data(a,8))[0]
def text_at(a):
 off=offset(a);end=b.index(0,off);claim(end-off<1024,'bounded C string');return b[off:end].decode()
at=32
for _ in range(struct.unpack_from('<I',b,16)[0]):
 cmd,n=struct.unpack_from('<II',b,at)
 if cmd==0x80000034:fix,size=struct.unpack_from('<II',b,at+8)
 at+=n
version,_,imports,symbols,count,fmt,sfmt=struct.unpack_from('<7I',b,fix);claim((version,fmt,sfmt)==(0,1,0),'original uncompressed format1 chained imports')
def binding(slot):
 val=u64(slot);claim(val>>63==1,'chained bind '+hex(slot));ordinal=val&0xffffff;claim(ordinal<count,'import ordinal bound');record=struct.unpack_from('<I',b,fix+imports+ordinal*4)[0];pos=fix+symbols+(record>>9);return b[pos:b.index(0,pos)].decode()
def imported(stub,slot,name):
 instruction(stub,'adrp','x16, #'+hex(slot&~4095));instruction(stub+4,'ldr','x16, [x16, #'+hex(slot&4095)+']');instruction(stub+8,'br','x16');claim(binding(slot)==name,'import identity '+name)
def selector(stub,slot,name):
 instruction(stub,'adrp','x1, #'+hex(slot&~4095));instruction(stub+4,'ldr','x1, [x1, #'+hex(slot&4095)+']')
 instruction(stub+8,'adrp','x16, #0x100731000');instruction(stub+12,'ldr','x16, [x16, #0x8b8]');instruction(stub+16,'br','x16');claim(binding(0x1007318b8)=='_objc_msgSend','selector dispatch via objc_msgSend')
 value=u64(slot);claim(not value>>63,'selector is rebased pointer');pointer=value&0xfffffffff;pointer=pointer if pointer>=0x100000000 else pointer+0x100000000;claim(text_at(pointer)==name,'selector identity '+name)
claim(text_at(0x10061e6e0)=='com.AudioKit.SuperJ6-Assets','default asset bundle identifier')
claim(binding(0x10077b718)=='_OBJC_CLASS_$_NSBundle','NSBundle class identity')
selector(0x1005db480,0x1007746b8,'bundleWithIdentifier:');selector(0x1005e1f40,0x100776168,'resourceURL')
for addr,mn,op in [(0x100195af8,'ldr','x0, [x20, #0x10]'),(0x100195afc,'cbz','x0, #0x100195b08'),(0x100195b08,'adrp','x8, #0x10061e000'),(0x100195b0c,'add','x8, x8, #0x6e0'),(0x100195b38,'bl','#0x1005db480'),(0x100196c44,'bl','#0x100195ae4'),(0x100196c90,'bl','#0x1005e1f40')]:instruction(addr,mn,op)
# Actual inline small String "PresetBanks" and its path append call.
for addr,mn,op in [(0x100196d30,'mov','x0, #0x7250'),(0x100196d34,'movk','x0, #0x7365, lsl #16'),(0x100196d38,'movk','x0, #0x7465, lsl #32'),(0x100196d3c,'movk','x0, #0x6142, lsl #48'),(0x100196d40,'mov','x1, #0x6b6e'),(0x100196d44,'movk','x1, #0x73, lsl #16'),(0x100196d48,'movk','x1, #0xeb00, lsl #48'),(0x100196d5c,'bl','#0x1005d7458'),(0x100196dbc,'bl','#0x1005d7f98')]:instruction(addr,mn,op)
imported(0x1005d7458,0x1007338f8,'_$s10Foundation3URLV22appendingPathComponentyACSSF')
imported(0x1005d7f98,0x100733d48,'_$sSo13NSFileManagerC10FoundationE10enumerator2at26includingPropertiesForKeys7options12errorHandlerSo21NSDirectoryEnumeratorCSgAC3URLV_SaySo16NSURLResourceKeyaGSgSo0M18EnumerationOptionsVSbAM_s5Error_ptcSgtF')
claim(text_at(0x10061fbc0)=='superj6.bank.json','bank filename suffix')
imported(0x1005d741c,0x1007338d0,'_$s10Foundation3URLV17lastPathComponentSSvg');imported(0x1005d7d4c,0x100732878,'_$sSS9hasSuffixySbSSF')
for addr,mn,op in [(0x100196e18,'adrp','x8, #0x10061f000'),(0x100196e1c,'add','x8, x8, #0xbc0'),(0x100196eb0,'bl','#0x1005d741c'),(0x100196ecc,'bl','#0x1005d7d4c'),(0x100196edc,'tbz','w25, #0, #0x100196f6c'),(0x100196ee4,'bl','#0x1005d74c4'),(0x100196ef8,'bl','#0x100195bac'),(0x100195e70,'bl','#0x1005d74f4'),(0x100195e88,'bl','#0x1000a3874'),(0x1000a39e8,'bl','#0x1000a3124'),(0x1000a34dc,'bl','#0x1002c9360')]:instruction(addr,mn,op)
imported(0x1005d74c4,0x100733940,'_$s10Foundation3URLV4pathSSvg');imported(0x1005d74f4,0x100733978,'_$s10Foundation4DataV10contentsOf7optionsAcA3URLVh_So20NSDataReadingOptionsVtKcfC')
# Capture once: archive identity and plist/bank member records derive from same bytes.
archive=args.ipa.read_bytes();archive_sha=hashlib.sha256(archive).hexdigest();claim(archive_sha=='dde4992f24f52d4390391069025b27092a2f327e3a53115bdd53a6ede0dffa8e','original archive identity')
with zipfile.ZipFile(io.BytesIO(archive))as z:
 prefix='Payload/SuperJ6.app/Frameworks/SuperJ6_Assets.framework/';info=plistlib.loads(z.read(prefix+'Info.plist'));claim(info['CFBundleIdentifier']==text_at(0x10061e6e0),'archive bundle identifier equals default loader literal')
 members=[n for n in z.namelist()if n.startswith(prefix+'PresetBanks/') and n.endswith('.superj6.bank.json')];claim(len(members)==13,'thirteen resources including separate User Bank')
 banks=[json.loads(z.read(n))for n in members];user=[v for v in banks if v['name']=='User Bank'];factory=[v for v in banks if v['name']!='User Bank']
 claim(len(user)==1 and len(user[0]['presets'])==1,'one separate User Bank init preset')
 claim(len(factory)==12 and sum(len(v['presets'])for v in factory)==640,'requested twelve-bank/640-preset population excludes User Bank init')
 result={'scope':__doc__,'binary_sha256':sha,'archive_sha256':archive_sha,'disassembler':dependency_receipt,'checks':checks,'bundle_identifier':info['CFBundleIdentifier'],'bank_resources':len(members),'requested_factory_banks':len(factory),'requested_factory_presets':sum(len(v['presets'])for v in factory),'separate_user_bank_presets':1,'bounded_finding':'The inspected uncached/no-override asset-bundle branch enumerates PresetBanks resource URLs with the original bank suffix, reads file Data and passes it through the previously inspected bank/preset JSON decoder. This does not close selected-preset/cache/alternate transformations before final controls.','original_execution':'NOT_RUN','S5':'FAIL','migration':'NOT_PROVEN','missing_proof':['Cache/override provenance and complete selected-preset path into the final control routine','Original-capable execution or equivalent complete source evidence for each legacy key and value']}
args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(checks),'fixed static/input checks; final routing NOT_PROVEN')
