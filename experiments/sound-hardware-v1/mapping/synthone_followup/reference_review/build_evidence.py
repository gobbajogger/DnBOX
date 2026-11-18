#!/usr/bin/env python3
"""Reference-side extractor. Writes only the owned mapping follow-up directory.

The implementer packet contains numerical observations and public control data;
this extractor and its original/source-aware inputs are not implementer inputs.
"""
from pathlib import Path
import csv, collections, hashlib, itertools, json, math, re, struct

ROOT=Path(__file__).resolve().parents[1]
RESEARCH=ROOT.parents[1]
RECON=RESEARCH/'sound_reference/DnBOX-Sound-694-factory-reconstruction-20261117-1612/sound-reconstruction-20261117'
FACTORY=RECON/'factory-full'
CLOSURE=RECON/'baseline/o2.a5c67ubr/closure'
INPUTS=FACTORY/'factory-inputs'
CORE=RESEARCH/'owner_evidence/sound-hardware-core-spike-v4/hardware-core-spike'
DELTA=RESEARCH/'owner_evidence/sound-classic-handoff-delta'
LANE=RESEARCH/'sound_lane'
SAFE=ROOT/'implementer_brief'
RESULT=ROOT/'results'
files={}

def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def observe(p):
    files[str(p.resolve())]={'sha256':sha(p),'bytes':p.stat().st_size}
    return p
def readjson(p): return json.loads(observe(p).read_text())
def save(p,data): p.write_text(json.dumps(data,indent=2,allow_nan=False)+'\n')
def f32(x): return struct.unpack('<f',struct.pack('<f',float(x)))[0]
def fromhex(x): return struct.unpack('>f',bytes.fromhex(x))[0]

descriptor=observe(INPUTS/'firmware/src/dsp/synthone_controls_data.inc').read_text()
pattern=r'\{(\d+),(\d+),0x([0-9a-f]+)u,0x([0-9a-f]+)u,0x([0-9a-f]+)u,"([^"]+)","([^"]+)"'
catalog=[]
selected_text=observe(CLOSURE/'controls/generated/parameter_map.cpp').read_text()
selected=set(map(int,re.search(r'selected_indices\[selected_count\] = \{([^}]+)',selected_text)[1].split(',')))
for m in re.finditer(pattern,descriptor):
    ident=int(m[1]); catalog.append({'id':ident,'name':m[6],
        'minimum':fromhex(m[3]),'default':fromhex(m[4]),'maximum':fromhex(m[5]),
        'min_default_max_f32_hex':[m[i] for i in (3,4,5)],
        'units':m[7] if 'native physical' not in m[7] else 'physical/native; see behavior brief',
        'smoothed_in_frozen_reference':ident in selected,
        'dependent_normalized_api':ident in (15,47,36,43,140,147)})
assert [d['id'] for d in catalog]==list(range(150)) and len(selected)==46
byid={d['id']:d for d in catalog}; defaults={d['id']:d['default'] for d in catalog}

cmdpath=FACTORY/'commands/ORIGINAL-PRESET-COMMANDS.json'
commands=readjson(cmdpath)
factory_result=readjson(FACTORY/'FACTORY-RESULT.json')
build_commands=observe(FACTORY/'factory-build-commands.json')
frontend_binary=observe(FACTORY/'commands/preset-commands.bin')
assert sha(build_commands)==factory_result['factory_commands_sha256']
assert sha(frontend_binary)==factory_result['frontend_commands_sha256']
packed=bytearray(struct.pack('<II',0x53315043,len(commands['cases'])))
for case in commands['cases']:
    packed.extend(struct.pack('<I',len(case['commands'])))
    for ident,value in case['commands']:packed.extend(struct.pack('<If',ident,value))
assert bytes(packed)==frontend_binary.read_bytes()
manifest=readjson(INPUTS/'assets/factory_banks/synthone/MANIFEST.json')
banks=[]
for entry in manifest['files']:
    p=INPUTS/'assets/factory_banks/synthone'/entry['name']; bank=readjson(p)
    assert sha(p)==entry['sha256'] and len(bank)==entry['count']
    banks.append({'name':entry['name'],'count':len(bank),'sha256':sha(p)})
assert sum(b['count'] for b in banks)==694

# Extract only public field identities, never engine implementation.
swift=observe(INPUTS/'tests/fixtures/synthone-factory/frontend/Preset.swift').read_text()
manager=observe(INPUTS/'tests/fixtures/synthone-factory/frontend/PresetDataManager.swift').read_text()
field_keys={m[1]:m[2] for m in re.finditer(r'(\w+)\s*=\s*dictionary\["([^"\n]+)"\]\s*as\?\s*Double\s*\?\?\s*p\(\.(\w+)\)',swift)}
field_keys.update(arpIsSequencer='arpIsSequencer',transpose='transpose')
loader=manager[manager.index('    func loadPreset()'):manager.index('    func saveValuesToPreset()')]
bindings=[(m[1],field_keys[m[2]]) for m in re.finditer(r's\.setSynthParameter\(\.(\w+),\s*(?:Double\()?activePreset\.(\w+)',loader)]
assert len(bindings)==96
byname={d['name']:d for d in catalog}
for name,key in bindings: byname[name]['preset_key']=key
for start,key in [(67,'seqPatternNote'),(83,'seqOctBoost'),(99,'seqNoteOn')]:
    for k in range(16):byid[start+k]['preset_array_key']=key;byid[start+k]['preset_array_index']=k

cases=commands['cases']; assert len(cases)==694
seen=collections.Counter();states=[];violations=[]
for case in cases:
    assert len(case['commands'])==144 and len({i for i,v in case['commands']})==144
    state=defaults.copy()
    for i,value in case['commands']:
        assert math.isfinite(value) and value==f32(value);seen[i]+=1;state[i]=value
        if not(byid[i]['minimum']<=value<=byid[i]['maximum']):
            violations.append({'bank':case['bank'],'preset_index':case['preset_index'],
                               'id':i,'name':byid[i]['name'],'command_value':value,
                               'min':byid[i]['minimum'],'max':byid[i]['maximum']})
    states.append(state)
for d in catalog:
    values=[s[d['id']] for s in states];d['factory_command_count']=seen[d['id']]
    d['factory_command_or_default_range']=[min(values),max(values)]
    d['factory_nonzero_count']=sum(v!=0 for v in values)
    d['factory_unique_command_or_default_values']=len(set(values))

conditions=[('bandlimit_disabled',lambda s:s[146]==0),
            ('lowpass_selected',lambda s:s[115]==0),
            ('oscillator1_saw_endpoint',lambda s:s[0]==1),
            ('oscillator2_volume_zero',lambda s:s[6]==0),
            ('sub_volume_zero',lambda s:s[7]==0),
            ('fm_volume_zero',lambda s:s[10]==0),
            ('noise_volume_zero',lambda s:s[12]==0),
            ('all_twelve_lfo_routes_off',lambda s:all(s[i]==0 for i in range(48,60))),
            ('arp_off',lambda s:s[62]==0),('poly_mode',lambda s:s[20]==0),
            ('glide_zero',lambda s:s[21]==0),
            ('dry_stereo_fx_controls',lambda s:(s[37]==0 or s[40]==0) and (s[41]==0 or s[44]==0) and s[116]==0 and s[35]==0 and s[121]==0 and s[34]>=44100)]
eligible=list(range(694));funnel=[]
for name,test in conditions:
    eligible=[k for k in eligible if test(states[k])]
    funnel.append({'condition':name,'individual_count':sum(test(s) for s in states),'cumulative_count':len(eligible)})
assert not eligible
coverage={'scope':'Static source-derived ordered command corpus; not Apple execution or candidate audio acceptance',
          'program_count':694,'bank_count':12,'banks':banks,
          'direct_bindings':96,'sequence_writes':48,'writes_per_program':144,
          'parameter_count':150,'smoothed_count':46,
          'unwritten_ids':[i for i in range(150) if not seen[i]],
          'exact_historical_patch_subset_factory_matches':0,
          'subset_filter_funnel':funnel,
          'pre_setter_out_of_metadata_range_count':len(violations),
          'pre_setter_out_of_metadata_range_by_id':dict(collections.Counter(v['id'] for v in violations)),
          'filter_types':dict(collections.Counter(s[115] for s in states)),
          'bandlimit':dict(collections.Counter(s[146] for s in states)),
          'unknown_or_nonDSP_field_counts':dict(collections.Counter(k for c in cases for k in c['preserved_nonDSP_or_unknown_fields'])),
          'raw_command_limits':['Values are before dependent setter and smoothing side effects.',
             'No generic 0..1 normalization. Six dependent IDs require their own contract.',
             'Preset data plus commands do not represent original Swift type-conversion execution.',
             'Zero intact factory matches does not imply that all subsets of their signal paths fail.']}
save(RESULT/'factory_coverage.json',coverage)
save(RESULT/'pre_setter_range_exceptions.json',violations)
save(SAFE/'s1_control_catalog.json',{'schema':'S1 public control identity and binary32 physical bounds v1','parameter_count':150,
    'do_not_use_superj6_ids':True,'units_warning':'Unqualified native values are not normalized UI pots.',
    'controls':catalog})

historical=[]
for label,path,expected in [('physical_gain',CORE/'physical-gain/SUMMARY.json',180),('filter_modulation',DELTA/'filter-modulation/SUMMARY.json',72)]:
    j=readjson(path);assert j['complete'] and len(j['rows'])==expected and j['accepted']==expected
    assert all(0<=r['rms_db']<=1 and 0<=r['spectral_db']<=3 for r in j['rows'])
    dims=('velocity','cutoff','resonance','note') if label=='physical_gain' else ('depth','cutoff','resonance','note')
    grid={d:sorted({r[d] for r in j['rows']}) for d in dims}
    assert len(set(tuple(r[d] for d in dims) for r in j['rows']))==math.prod(len(v) for v in grid.values())==expected
    historical.append({'name':label,'rows':expected,'grid':grid,'threshold_rms_db':1,'threshold_spectral_db':3,
                       'maximum_rms_db':j['maximum_rms'],'maximum_spectral_db':j['maximum_spectral'],
                       'scope':'Historical reference-aware prototype, not new candidate. Original DSP implementation not eligible for reuse.'})
save(RESULT/'historical_matrix_audit.json',historical)

vel=[{'velocity':v,'linear_velocity_fraction':f32(v/127),'s1_squared_fraction':f32(f32(v/127)**2)} for v in (1,20,64,100,127)]
cutoffs=[]
for base,depth,env in itertools.product((64,500,2000,8000,22050),(0,.5,1,1.2),(0,.25,.4,.5,1)):
    value=max(64,min(22050,base*(1-depth*(1-env))))
    cutoffs.append({'base_hz':base,'depth':depth,'filter_envelope_level':env,'target_hz':value})
save(SAFE/'behavior_fixtures.json',{'schema':'Numeric S1 behavior observations and scalar contract v1',
    'sample_rate':44100,'velocities':vel,'filter_cutoff_targets':cutoffs,
    'historical_common_patch':{'frames_per_case':88192,'warmup_frames':176448,'note_on_frame':0,'note_off_frame':66112,
       'oscillator1_morph':1,'oscillator1_level':1,'balance':.5,'oscillator2_level':0,'sub_level':0,'fm_level':0,'noise_level':0,
       'bandlimit_enable':0,'filter_type':0,'parameter_smoothing_half_time_s':1e-6,'amp_ADSR':[.05,.005,.8,.05],
       'filter_ADSR_physical_gain':[.05,.05,1,.5],'filter_ADSR_modulation':[.2,.3,.4,.5],
       'velocity_modulation_matrix':100,'comparison_gain':1,'master_and_FX':'excluded from dry comparison',
       'tuning':'Reference owner supplies the exact per-note frequency; do not fit it per output or silently change tuning.'},
    'oscillator_endpoint_identity':{'s1_id':0,'s1_value':1,'waveform':'raw saw when ID146=0','candidate_waveform_value':2/3},
    'historical_dry_scenes':historical,
    'signal_order':['oscillator components with squared velocity and component gains','component sum and amplitude envelope','selected voice filter','voice sum','separate post-voice FX and master'],
    'required_negative_controls':['Treating velocity as linear','Treating S1 waveform1 as candidate square','Clamping depth1.2 to1','Additive rather than multiplicative filter envelope','Moving VCA after filter']})

# The fresh implementer gets the numeric component observations, never adsr.c.
bycase=collections.defaultdict(list)
for r in csv.DictReader(observe(RESULT/'s1_envelope_full.csv').open()):
    bycase[int(r['case'])].append(f32(r['output']))
assert len(bycase)==3 and all(len(v)==88200 for v in bycase.values())
binary=SAFE/'s1_envelope_observations.f32le'
with binary.open('wb') as f:
    for c in range(3):f.write(struct.pack('<88200f',*bycase[c]))
envelope_cases=[]
for c,settings in enumerate(((.05,.005,.8,.05),(.2,.3,.4,.5),(.0005,.005,0,.004))):
    values=bycase[c];peak=max(range(len(values)),key=values.__getitem__)
    envelope_cases.append({'case':c,'settings_ADSR':[f32(x) for x in settings],
        'frames':88200,'byte_offset':c*88200*4,'note_on_frame':0,'note_off_frame':44100,
        'peak_frame':peak,'peak_level':values[peak],
        'selected_observations':{str(k):values[k] for k in (0,1,16,22,441,2204,2205,4410,8819,8820,22050,44099,44100,44101,46305,66150,88199)}})
save(SAFE/'envelope_observations.json',{'scope':'Fresh frozen original S1 ADSR component execution. No complete app or voice lifecycle claim.',
    'format':'Little-endian float32; three concatenated case-major sequences; one scalar sample per audio frame',
    'sample_rate':44100,'binary_file':binary.name,'sha256':sha(binary),'frames':264600,
    'cases':envelope_cases,'limits':['Separate from SuperJ6 AHDSHR captures and fits.',
    'No original engine source or recurrence is included in the implementer packet.',
    'Static ADSR values only; live automation, retrigger, event ordering, and full voice retirement remain unqualified.',
    'Both amp and filter reference paths use this ADSR component, but the filter-to-frequency route needs independent verification.',
    'Two-second traces do not establish that release reaches exactly zero.']})

for rel in ['CLASSIC-HANDOFF.md','filter-modulation/dry-reference.cpp','filter-modulation/law-test.c','filter-modulation/law-red.log','filter-modulation/law-green.log']:
    observe(DELTA/rel)
for rel in ['PROGRESS-V3.md','metrics.py','physical-gain/dry-reference.cpp','env-result.log','env-test.c','prepared-core/result.log']:
    if (CORE/rel).exists():observe(CORE/rel)
for rel in ['experiment/probe/portable_voice.cpp','experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe/modules/adsr.c','master/AudioKitSynthOne/DSP/S1Parameter.h']:
    observe(CLOSURE/rel)
observe(INPUTS/'tests/fixtures/synthone-factory/frontend/preset_audio.cpp')
observe(INPUTS/'tests/fixtures/synthone-factory/fullfx/source_fx.hpp')
for rel in ['experiments/original_synth/chicago_mp4.c','experiments/original_synth/chicago_mp4.h','experiments/original_synth/miami_ms20.c','experiments/original_synth/miami_ms20.h','experiments/original_synth/chicago_voice.c','firmware/src/dsp/juno6.c','firmware/src/dsp/master.c','firmware/src/dsp/master.h','firmware/src/dsp/drum909_dsp.h','firmware/src/dsp/busfx.h','firmware/src/app/original_sound.c']:
    observe(LANE/rel)
for rel in ['src/dnbx_core.c','src/dnbx_core.h','src/fastmath.h','evidence/source_provenance.json']:
    observe(RESEARCH/'hardware_candidate'/rel)
save(RESULT/'input_provenance.json',{'upstream_s1_pin':manifest['source_pin'],
    'reference_only':'Original source is not included in the implementer packet. No original algorithms were copied into candidate code here.',
    'files':files})
save(RESULT/'mapping_checks.json',{'passed':True,'checks':['150 consecutive control IDs','46 smoothed IDs','12 exact bank SHA256 matches and694 entries',
    '694 command sequences each144 unique ordered writes','Factory build andfrontend binary SHA match original694 receipt; JSON reserialization matches exact binary','96 public field bindings+48 sequence fields',
    '180 historical rows complete Cartesian grid andwithin predeclared gates','72 dynamic filter rows complete Cartesian grid andwithin gates',
    '0 complete factory programs inhistorical single-saw subset','264600 finite numeric envelope observations'],
    'pre_setter_range_exceptions':len(violations),'candidate_audio_acceptance':False,'production_files_written':0})
print(json.dumps({'catalog':len(catalog),'banks':len(banks),'programs':len(cases),'physical_subset_factory_matches':0,
    'range_exceptions':len(violations),'historical_rows':252,'envelope_observations':264600}))
