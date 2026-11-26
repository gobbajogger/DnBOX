from pathlib import Path
import json, math, tarfile, hashlib
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
W=Path('/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src/dsp')
P=Path('sj6-cross-preset')
a=P/'SJ6-Three-Factory-Presets-original-AUv3.tgz'
assert sha(a)=='0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603'
with tarfile.open(a) as t: d=json.load(t.extractfile('./program-2-dry/capture.json'))
assert d['active']==d['after']
r={int(v['address']):v['value'] for v in d['active']}
q=P/'sub-range.private-output/2-dry.private.txt'
lines=q.read_text().splitlines();controls={int(i):float(v) for i,v in map(str.split,lines[2:2+int(lines[1])])}
assert 0<r[202]<1 and 0<controls[202]<1 and abs(controls[202]-r[202])<1e-7
assert [r[k] for k in (7,8,131,136,180,201,209)]==[1,0,0,0,1,0,1]
anchors={}
for name,need in [('dnbx_core.c',['s->sj6_sub_gain=(.808f/127.f)*(float)code[1]', 'if (!retrigger) memset(v,0,sizeof *v);','x+=s->sj6_sub_gain*sj6_synthesize_sub','base+env*amount','mix[l]+=velocity*amp*sj6_ota_filter']),('juno6_factory.inc',['juno6_sj6_panel_cutoff_hz(float raw,unsigned mode)','case 129:{','juno6_aux_core_apply(','int rc=render_typed_span_raw','params.cutoff_hz=saved_cutoff[0]'])]:
 f=W/name; lines=f.read_text().splitlines()
 anchors[name]=dict(sha=sha(f),sites={v:[i+1 for i,l in enumerate(lines) if v in l] for v in need})
 assert all(anchors[name]['sites'].values())
cand=[30000.0078125,8510.4130859375]
assert [min(.75,2*math.pi*fc/44100) for fc in cand]==[.75,.75]
print(json.dumps(dict(authorized_fixture_sha=sha(a),typed_fixture_sha=sha(q),source_anchors=anchors,admitted=False,owner_first_failure='Authored and typed fixture #202 nonzero; routed compact Sub injected into Main before the OTA site',original_public_202=r[202],compact_public_202=controls[202],clamp_alias=cand,scope='cold registered Native lab only; no activation proposed'),indent=2))
