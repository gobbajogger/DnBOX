from pathlib import Path
import json,re,hashlib
root=Path(__file__).resolve().parent.parent
out=Path(__file__).resolve().parent
fw=root/'full-fw-integ'
closure=root/'s1-fix-data/source-fixtures'
up=closure/'closure/master/AudioKitSynthOne/DSP'
ui=root/'ui-1340-reconciled'
e={n:next(ui.glob(pattern)) for n,pattern in {
'stock':'synthone-product-control-audit','osc':'synthone-oscillator-editor.*',
'filter':'synthone-filter-editor.*','prep':'synthone-modulation-prep.*'}.items()}
def path(p): return str(p.relative_to(root))
def at(p,line): return f'{path(p)}:{line}'
def loc(p,expr):
    return [at(p,i) for i,line in enumerate(p.read_text().splitlines(),1) if re.search(expr,line)]
enum=up/'S1Parameter.h'
orig={int(i):(n,lin) for lin,s in enumerate(enum.read_text().splitlines(),1) if (m:=re.search(r'^\s*(\w+)\s*=\s*(\d+)',s)) for n,i in [m.groups()] if n!='S1ParameterCount'}
assert set(orig)==set(range(150))
kern=up/'Kernel'
s1=fw/'firmware/src/dsp/s1c'
part=fw/'firmware/src/app/s1c_part.c'
app=fw/'firmware/src/app/ui.c'
ds=s1/'s1c.c'
code=ds.read_text()
decl=re.search(r'enum \{ I1,.+?KTRK \};',code,re.S)[0]
aliases={}; val=0
for token in decl[len('enum { '):-3].split(','):
    item,sep,value=token.strip().partition(' = ')
    if sep:
        a,b=value.split(' + '); val=next(k for k,x in aliases.items() if x==a)+int(b)
    aliases[val]=item; val+=1
assert {aliases[i] for i in (0,15,47,115,138,145,149)}=={'I1','L1R','L2R','FTYPE','DIRES','BLO','KTRK'}
f=closure/'commands/ORIGINAL-PRESET-COMMANDS.json'
factory=json.loads(f.read_bytes())['cases']
ids=[i for i,_ in factory[0]['commands']]
assert len(factory)==694 and len(ids)==144 and len(set(ids))==144
assert all([i for i,_ in x['commands']]==ids for x in factory)
types={}
def put(group,nums,status,note=''):
    for n in nums:
        assert n not in types,n
        types[n]=(group,status,note)
put('oscillator',list(range(13))+[30,31,146],'dsp_control')
put('filter_envelopes',[16,17,19,*range(22,30),115,149],'dsp_control')
put('lfo_open',[13,15,45,47,139],'assigned_lfo_work','Prep proves shape/sync law and storage capacity, not an editable implementation.')
put('modulation_followup',[14,46,*range(48,60)],'dsp_control','LFO amplitudes and 12 destination selectors; not the five controls in the audited LFO prep.')
put('unused',[18,33,145],'not_product_control','Original explicitly unused/deprecated; do not expose as a missing panel control.')
put('voice_tuning',[20,21,120,141,142,143,144,148],'dsp_or_app_control')
put('performance',[140],'performance_input','Pitch bend event, not a saved knob; separately audit configurable range.')
put('wet_fx',[34,*range(35,45),*range(116,120),121,137,138],'dsp_control')
put('output',[32,*range(122,137)],'dsp_control','Synth One voice/output/reverb compressors are not the DnBOX MIX/master target.')
put('arp_globals',[*range(60,67),147],'shared_different_semantics','Shared DnBOX ARP, not the original embedded sequencer; exact original recall/time law not proved.')
put('step_pitch',range(67,83),'sequencer_field')
put('step_octave',range(83,99),'sequencer_field')
put('step_gate',range(99,115),'sequencer_field')
assert set(types)==set(orig)
stock={}
stock_order=[(2,'OSCILLATOR',5),(5,'OSCILLATOR',6),(16,'FILTER',5),(17,'FILTER',6),(19,'FILTER',7),
  *[(22+i,'FILTER ADSR',5+i) for i in range(4)],*[(26+i,'AMP ADSR',5+i) for i in range(4)]]
for sid,page,k in stock_order:
    match=re.search(r'\{"([^"]+)",[^\n]+, '+str(sid)+r', X0X_U_',part.read_text())
    assert match,sid
    stock[sid]={'page':page,'knob':k,'label':match[1],'proof':'stock_patch'}
stock[34]={'page':'EFFECT','knob':5,'label':'CRUSH RATE','proof':'stock_crush'}
osc={sid:{'page':p,'knob':k,'label':n,'proof':'oscillator_candidate'} for sid,p,k,n in [
(0,'OSCILLATOR',7,'OSC 1 MORPH'),(1,'OSCILLATOR',8,'OSC 2 MORPH'),
(6,'OSC MIX',5,'OSC 2 LEVEL'),(7,'OSC MIX',6,'SUB LEVEL'),(8,'OSC MIX',7,'SUB OCTAVE'),(9,'OSC MIX',8,'SUB WAVE'),
(10,'FM / NOISE',5,'FM LEVEL'),(11,'FM / NOISE',6,'FM INDEX'),(12,'FM / NOISE',7,'NOISE LEVEL')]}
flt={115:{'page':'FILTER MODE','knob':5,'label':'FILTER TYPE','proof':'filter_candidate'},
149:{'page':'FILTER MODE','knob':6,'label':'ADSR PITCH TRACK','proof':'filter_candidate'}}
exact={
18:'Upstream annotated hard-coded to 1. Compact uses startup smoothing; not an authored editor.',
33:'Upstream getter metadata only; declared unused. Do not conflate with editable sample rate (34).',
145:'Upstream deprecated bandlimit override. Distinct from active bandlimit enable (146).',
138:'Original setup sets delay input ladder resonance; compact alias occurs only in declaration and both ladder branches pass 0. Compact per-control runtime missing.',
141:'Original configurable bend lower endpoint; compact PBMIN alias unused in s1c.c. Audit actual host bend curve before posting parity.',
142:'Original configurable bend upper endpoint; compact PBMAX alias unused in s1c.c. Audit actual host bend curve before posting parity.',
143:'Compact frontend stores/truncates A4, but voice uses fixed note_hz table; no active render tuning route shown. Upstream selected tuning/controller and master sets are separate.',
60:'Original enum/order Up, UpDown, Down; DnBOX shared ARP mode adds SEQ; do not equate numeric values.',
61:'Original interval feeds original arp step order; shared DnBOX ARP SEQ interval is not validated as same law.',
62:'Shared ON is available; original pack arp enable is not imported into shared ARP at factory selection.',
63:'Shared ARP range is analogous; original bounds/order and recalled state not matched.',
64:'Original arp rate is a per-patch BPM and dependency for LFO, pan and delay sync; shared speed is transport division.',
65:'Original arp-is-sequencer flag maps conceptually to shared SEQ mode; not a native original flag route.',
66:'Shared ARP SEQ LENGTH exists; original authored step-count import not tested.',
147:'Original tempo multiplier shapes original seq duration; compact frontend can store/quantize it, but built with embedded sequencer disabled.',
140:'MIDI/physical performance path, not a saved knob. Compact s1_part_param applies fixed ±2 semitones via the program multiplier; this is not original configurable bend endpoints (141/142).',
31:'Compact oscillator and fixed bend both use this multiplier; not present in the 144 compiled per-program command IDs. Only default/performance evidence, not authored persistent editor.',
144:'Original portamento half-time DSP field has a compact coefficient setter; not in the 144 compiled per-program command IDs. Distinct from the glide editor domain.'}
proofs={
'none':{'owners':'not established per control','persistence':'no editor round trip established','factory':'ordered input transport is not audibility or edit parity'},
'stock_patch':{'owners':'A + B in stock typed patch/panel fixtures (existing evidence)','persistence':'original 32-byte typed Sound cell override, existing part ownership/refusal/reload tests','factory':'14 edit claims not an all-controls DSP parity statement; exact untouched PCM preserved by later candidate suites','refs':[path(e['stock']/'README.md')]},
'stock_crush':{'owners':'per-part stock sound route; owner aggregate from stock audit','persistence':'typed original crush override in sound cell','factory':'source ID 34 is distinct from unused depth; no per-program edit sweep','refs':[path(e['stock']/'README.md')]},
'oscillator_candidate':{'owners':'A/B edits change held PCM, 194 checks; authenticated programs 129 & 694 in 28 factory edit checks','persistence':'11-byte T_303 rows 6/7, legacy marker/refusal, typed note lock, project reload, preset reset','factory':'all 694 + Init untouched PCM identical vs baseline; edited exact restoration sampled at 129 and 694','refs':[path(e['osc']/x) for x in ['README.md','green.log','factory-editor.log','factory.log']]},
'filter_candidate':{'owners':'A/B mode and tracking effects/peer isolation; 33,234 checks','persistence':'extends side records to 13 bytes, note locks and project round trip; older reader refusal tested','factory':'694 + Init unchanged PCM; original filter targets restored after overrides in all 695','refs':[path(e['filter']/x) for x in ['HANDOFF.md','factory-values.log','factory-pcm.log']]},
'arp_shared':{'owners':'A/B shared engine and UI in prior receipts, not original preset-law parity','persistence':'shared engine per-part ARP state and steps; project tests not proof original preset import','factory':'original embedded sequencer disabled with S1C_NO_SEQ=1','refs':[path(e['stock']/'README.md'),path(e['osc']/'arp-compact.log')]},
'lfo_prep_only':{'owners':'no product A/B owner-edit proof in this checkpoint','persistence':'storage capacity model only; restoration must preserve rates, pan and delay','factory':'all 694 shape/rate/sync words inspected; 1197 dependency comparisons; no editor assertion','refs':[path(e['prep']/x) for x in ['HANDOFF.md','original-rate-oracle.log','factory-lfo-words.log']]}}
arpanalog={60:'MODE',61:'ARP SEQ INTERVAL (analog only)',62:'ON',63:'RANGE',64:'SPEED',65:'SEQ mode',66:'ARP SEQ LENGTH'}
rows=[]
for id,(identity,line) in orig.items():
    group,kind,note=types[id]
    au=[at(enum,line)]
    descriptor=loc(kern/'S1DSPKernel.hpp',r'\{\s*'+re.escape(identity)+r',')
    run=[]
    for file in ['S1DSPKernel+process.mm','S1DSPKernel+getSetSynthParameters.mm','S1DSPKernel.mm','S1DSPKernel+MIDI.mm']:
        run+=loc(kern/file,r'\b'+identity+r'\b')[:3]
    auth={'enum':au[0],'descriptor_refs':descriptor,'runtime_or_callback_refs':run,
          'ios_editor_inventory':'not established by kernel metadata'}
    candidate=osc.get(id) or flt.get(id)
    if id in stock:
        panel=stock[id]; back='stock_direct'; evidence=panel['proof']
    elif candidate:
        panel=None;back='host_qualified_candidate';evidence=candidate['proof']
    elif group in ('step_pitch','step_octave','step_gate') or id in arpanalog:
        panel={'page':'ARPEGGIATOR/ARP SEQ','label':arpanalog.get(id,'16 pads: '+group)};back='shared_analogue_only';evidence='arp_shared'
    elif group=='lfo_open':
        panel=None;back='assigned_lfo_work_not_counted';evidence='lfo_prep_only'
    else:
        panel=None;back={'unused':'excluded_explicitly','performance':'performance_only'}.get(group,'not_exposed');evidence='none'
    alias=aliases.get(id)
    runtime=[]
    if alias:
        runtime=[x for x in loc(ds,r'\b'+alias+r'\b') if int(x.rsplit(':',1)[1])>60][:12]
    if id in [15,36,43,47,64,139,140,143,147]:
        front=s1/'s1c_front.c'
        runtime+=loc(front,r'\bF_(?:L1R|PANF|DLT|L2R|ARPRATE|TSYNC|PB|A4|TMULT)\s*=\s*'+str(id)+r'\b')
    if id == 140:
        runtime+=loc(part,r'\+\-2 semitones')
    if group.startswith('step_') or group=='arp_globals':
        rt='shared scheduler; embedded engine sequencer not compiled in production'
    elif group=='unused':
        rt='explicit source non-product'
    elif id in (138,141,142,143):
        rt='missing or stored-only compact per-control behavior'
    elif id==140:
        rt='compact wrapper uses fixed ±2 semitone performance span'
    else:
        rt='compact source reference present' if runtime else 'compact application/runtime not established by this audit'
    rows.append({'id':id,'original':identity,'family':group,'kind':kind,'authority':auth,
                 'stock_panel':panel,'checkpoint_candidate':candidate,'edit_state':back,
                 'compact_runtime':{'description':rt,'symbol':alias,'references':runtime},
                 'proof_set':evidence,'factory_commands':{'ordered_input_count':694 if id in ids else 0,
                     'distinct_authored_values':len({dict(x['commands'])[id] for x in factory}) if id in ids else 0,
                     'evidence_scope':'authenticated command inputs (distinct JSON command values); not individual native render/edit parity'},
                 'note':exact.get(id,note)})
extras=[
 ('DNBOX_FILTER_LINK','DnBOX-only control',{'page':'FILTER','knob':8,'proof':'separate_host_patch'},
  'Authenticated original app equivalence not established. Host linked both owners; unlink restores independent envelope.',
  path(e['stock']/'README.md')),
 ('ORIGINAL_MOD_WHEEL_ROUTE','App control authenticated outside enum',None,
  'Original callback routes wheel 0 cutoff, 1 LFO1 rate, 2 LFO2 rate; selector absent in stock. Keep controller routing separate from saved patch.',
  path(e['stock']/'README.md')),
 ('ORIGINAL_MASTER_TUNING_SETS','App control authenticated outside enum',None,
  'Original loader restores selectable master tuning sets; no full native tuning workflow. Exact original controller set not enumerated.',
  path(e['stock']/'README.md')),
 ('ORIGINAL_HOLD','App behavior not independently established',None,
  'Storage key isHoldMode is not proof of original runtime reset/recall; Sustain handler separately authenticated.',
  path(e['stock']/'README.md')),
 ('MIDI_SUSTAIN','Performance input outside enum',None,
  'Authenticated original sustainer callback and existing DnBOX part input; not a panel sound editor.',
  path(e['stock']/'README.md')),
 ('ORIGINAL_PRESET_BANK_PICKER','Preset workflow outside enum',None,
  '694 + Init packed identities are not proof of native bank chooser or original iOS browser inventory.',
  path(root/'bank-selector-audit/HANDOFF.md')),
 ('FULL_IOS_UI_INVENTORY','Unavailable evidence',None,
  'Authenticated closure lacks complete iOS controllers and screen inventory; no manual denominator or original iOS playback assumed.',
  path(e['stock']/'README.md'))]
report={'schema':'dnbox-s1-control-census/1','sources':{
 'original_commit':'6466a3715c96b7ecf1dd255a218cbd571408a314',
 'stock_audit_commit':'68be4d44edafbb2c41dcb5e7c773ac10bf00cf1f','pinned_compact_reference':'c4c5ccf13999204ea9e82dcb914ce0a9b3b5334f',
 'original_source_receipt':path(e['stock']/'source-receipt.json'),'commands_sha256':hashlib.sha256(f.read_bytes()).hexdigest(),
 'checkpoint_rule':'host-only checkpoints not claimed integrated, native-qualified, or a real iOS panel inventory'},
 'proof_sets':proofs,'stable_original_ids':rows,'outside_original_enum':[
 {'key':a,'class':b,'panel':c,'note':d,'source_ref':e} for a,b,c,d,e in extras]}
(out/'controls.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n')
from collections import Counter
print('rows',len(rows),'extra',len(extras),'state',Counter(x['edit_state'] for x in rows))
print('144 command columns; excluded IDs',sorted(set(orig)-set(ids)))
