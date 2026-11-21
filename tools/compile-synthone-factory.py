#!/usr/bin/env python3
# Source-derived host test command generator, not an Apple runtime/preset implementation.
from pathlib import Path
import json,re,struct,hashlib,argparse
repo=Path(__file__).resolve().parents[1];parser=argparse.ArgumentParser();parser.add_argument('--out',type=Path,required=True);args=parser.parse_args();r=args.out;r.mkdir(parents=True,exist_ok=True);root=repo/'tests/fixtures/synthone-factory/frontend';p=(root/'Preset.swift').read_text();m=(root/'PresetDataManager.swift').read_text();load=m[m.index('    func loadPreset()'):m.index('    func saveValuesToPreset()')];fields={}
for z in re.finditer(r'(\w+)\s*=\s*dictionary\["([^"\n]+)"\]\s*as\?\s*Double\s*\?\?\s*p\(\.(\w+)\)',p):fields[z[1]]={'key':z[2],'default_parameter':z[3],'type':'Double'}
descriptor=(repo/'firmware/src/dsp/synthone_controls_data.inc').read_text()
rows=[{'default_f32_hex':m[1]} for m in re.finditer(r'\{\d+,\d+,0x[0-9a-f]+u,0x([0-9a-f]+)u,',descriptor)]
assert len(rows)==150

names={m[2]:int(m[1]) for m in re.finditer(r'\{(\d+),\d+,0x[0-9a-f]+u,0x[0-9a-f]+u,0x[0-9a-f]+u,"([^"]+)"',descriptor)}
assert len(names)==150,len(names)
def default(name):return struct.unpack('>f',bytes.fromhex(rows[names[name]]['default_f32_hex']))[0]
fields['arpIsSequencer']={'key':'arpIsSequencer','default_parameter':'arpIsSequencer','type':'Bool'};fields['transpose']={'key':'transpose','default_parameter':'transpose','type':'Int'}
ordered=list(re.finditer(r's\.setSynthParameter\(\.(\w+),\s*(?:Double\()?activePreset\.(\w+)',load));assert len(ordered)>80
cases=[]
corpus=repo/'assets/factory_banks/synthone'
bank_manifest=json.loads((corpus/'MANIFEST.json').read_text())
for item in sorted(bank_manifest['files'],key=lambda x:x['name']):
 bank=corpus/item['name']
 assert bank.name==item['name'] and bank.name!='MANIFEST.json'
 assert hashlib.sha256(bank.read_bytes()).hexdigest()==item['sha256']
 for index,raw in enumerate(json.loads(bank.read_text())):
  commands=[];used=set();missing=[]
  for z in ordered:
   parameter,field=z[1],z[2];assert parameter in names,(parameter,field);assert field in fields,(parameter,field);f=fields[field];key=f['key'];value=raw.get(key,default(f['default_parameter']))
   if key not in raw:missing.append(key)
   if f['type']=='Bool':assert isinstance(value,(bool,int,float));value=1.0 if value else 0.0
   else:assert isinstance(value,(float,int))and not isinstance(value,bool),(bank.name,index,key,value)
   value=struct.unpack('<f',struct.pack('<f',float(value)))[0];assert value==value and abs(value)!=float('inf');commands.append([names[parameter],value]);used.add(key)
   if parameter=='arpSeqTempoMultiplier':
    for key,stem,kind in [('seqPatternNote','sequencerPattern00','int'),('seqOctBoost','sequencerOctBoost00','bool'),('seqNoteOn','sequencerNoteOn00','bool')]:
     arr=raw.get(key, [0]*16 if kind=='int'else[False]*16 if key=='seqOctBoost'else[True]*16);assert isinstance(arr,list)and len(arr)==16,(bank.name,index,key);used.add(key)
    for i in range(16):
     for key,stem,kind in [('seqPatternNote','sequencerPattern00','int'),('seqOctBoost','sequencerOctBoost00','bool'),('seqNoteOn','sequencerNoteOn00','bool')]:
      arr=raw.get(key, [0]*16 if kind=='int'else[False]*16 if key=='seqOctBoost'else[True]*16);value=arr[i];assert isinstance(value,bool)if kind=='bool'else isinstance(value,int)and not isinstance(value,bool);commands.append([names[stem]+i,float(value)])
  cases.append({'bank':bank.name,'preset_index':index,'name':raw.get('name'),'uid':raw.get('uid'),'source_sha256':hashlib.sha256(bank.read_bytes()).hexdigest(),'commands':commands,'defaulted_keys':sorted(set(missing)),'preserved_nonDSP_or_unknown_fields':sorted(set(raw)-used),'app_settings':{'freezeDelay':False,'freezeReverb':False,'freezeArpRate':False,'freezeArpSeq':False,'saveTuningWithPreset':False},'limits':['Known valid scalar/array JSON shapes only; AppleFoundation cast not executed','Original UI/tuning/hold/modwheel/unknown fields preserved externally, not falsely mapped to DSP','No original preset bytes edited; no invented aliases for fmMod/rez/autoPanRate','Original sequence per-step write order; resetSequencer after apply']})
(r/'ORIGINAL-PRESET-COMMANDS.json').write_text(json.dumps({'source':'6466a3715c96b7ecf1dd255a218cbd571408a314','preset_swift_blob':'ca03ec2995b96b7fb427a9eea52d9f7ae00c9863','manager_swift_blob':'9077cc36192f902f56620c35ae16ba5e57abeedc','direct_source_bindings':len(ordered),'cases':cases},indent=2)+'\n');print(json.dumps({'real_presets':len(cases),'direct_bindings':len(ordered),'commands_per_preset':sorted(set(len(x['commands'])for x in cases)),'source_not_run':'Swift runtime type conversion not executed; actual original JSON only','next':'Actual source-control/backend apply and fullFX musical comparison'}))

with (r/'preset-commands.bin').open('wb') as f:
 f.write(struct.pack('<II',0x53315043,len(cases)))
 for case in cases:
  f.write(struct.pack('<I',len(case['commands'])))
  for ident,value in case['commands']:f.write(struct.pack('<If',ident,value))
