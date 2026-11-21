#!/usr/bin/env python3
"""Validate only the portable behavior packet. Does not read original source."""
from pathlib import Path
import hashlib,json,math,struct
root=Path(__file__).resolve().parent
catalog=json.loads((root/'s1_control_catalog.json').read_text())['controls']
fixtures=json.loads((root/'behavior_fixtures.json').read_text())
meta=json.loads((root/'envelope_observations.json').read_text())
assert len(catalog)==150 and [c['id'] for c in catalog]==list(range(150))
assert sum(c['smoothed_in_frozen_reference'] for c in catalog)==46
assert {c['id'] for c in catalog if c['dependent_normalized_api']}=={15,47,36,43,140,147}
assert catalog[19]['maximum']>1.19 and catalog[16]['maximum']==22050
for c in catalog:
    for key,value in zip(('minimum','default','maximum'),c['min_default_max_f32_hex']):
        assert c[key]==struct.unpack('>f',bytes.fromhex(value))[0]
for row in fixtures['filter_cutoff_targets']:
    wanted=max(64,min(22050,row['base_hz']*(1-row['depth']*(1-row['filter_envelope_level']))))
    assert abs(wanted-row['target_hz'])<1e-8
assert fixtures['oscillator_endpoint_identity']['s1_value'] != fixtures['oscillator_endpoint_identity']['candidate_waveform_value']
negative_controls=0
for row in fixtures['velocities']:
    if row['velocity'] not in (0,127):
        assert abs(row['linear_velocity_fraction']-row['s1_squared_fraction'])>1e-3
        negative_controls+=1
row=next(r for r in fixtures['filter_cutoff_targets'] if r['base_hz']==2000 and r['depth']==1.2 and r['filter_envelope_level']==.4)
assert abs(row['target_hz']-560)<1e-8
wrong_clamped_depth=2000*(1-1*(1-.4))
assert abs(wrong_clamped_depth-row['target_hz'])==240
negative_controls+=1
data=(root/meta['binary_file']).read_bytes()
assert hashlib.sha256(data).hexdigest()==meta['sha256'] and len(data)==meta['frames']*4
values=struct.unpack('<'+str(meta['frames'])+'f',data)
assert all(math.isfinite(x) for x in values)
for case in meta['cases']:
    start=case['byte_offset']//4
    for frame,value in case['selected_observations'].items():assert values[start+int(frame)]==value
    assert case['peak_frame']>case['settings_ADSR'][0]*meta['sample_rate']
    assert values[start]!=values[start+1]
negative_controls+=3
print(json.dumps({'passed':True,'controls':len(catalog),'filter_targets':len(fixtures['filter_cutoff_targets']),
                  'envelope_samples':len(values),'negative_controls':negative_controls,
                  'scope':'Packet integrity and numerical contract only; no candidate DSP acceptance'}))
