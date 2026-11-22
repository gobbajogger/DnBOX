#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Replay exact original Synth One step-sequencer events in the firmware fixture."""
from pathlib import Path
import argparse
import gzip
import hashlib, importlib.util, json, struct, subprocess
import numpy as np

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("source_tree", type=Path)
parser.add_argument("verified_factory_build", type=Path)
parser.add_argument("panel_targets", type=Path)
parser.add_argument("output", type=Path)
args = parser.parse_args()
root, pinned, targets, work = (
    path.resolve()
    for path in (args.source_tree, args.verified_factory_build, args.panel_targets, args.output)
)
source = "bb836379"
work.mkdir()
assert hashlib.sha256(targets.read_bytes()).hexdigest() == (
    "d72756ce2b5898b275e65f5b301527f2e687eb7df6c1c0ea0a8c3d5967237833"
)
bench='experiments/sound-hardware-v1/synthone-factory-projection'
def git_file(p): return subprocess.check_output(['git','-C',root,'show',source+':'+p],text=True)
def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def once(s,a,b):
    assert s.count(a)==1,a
    return s.replace(a,b)

# Use the existing verified row scorer with its expected repository-root cwd.
import os
os.chdir(root)
spec=importlib.util.spec_from_file_location('replay',root/bench/'replay.py')
replay=importlib.util.module_from_spec(spec)
spec.loader.exec_module(replay)

cpp=git_file(bench+'/original.cpp')
cpp=once(cpp,'    float *left{}, *right{}, mono{};',
         '    float *left{}, *right{}, mono{};\n    unsigned frame{};')
cpp=once(cpp,'state.lfo2_0_1, state.lfo2_1_0);','state.lfo2_0_1, state.lfo2_1_0);\n    ++out.frame;')
cpp=once(cpp,'CHECK(argc == 6);','CHECK(argc == 7);')
cpp=once(cpp,'    Output out;', '''    Output out;
    FILE *events = std::fopen(argv[6], "wb");
    CHECK(events && pair.o.parameters[arpIsOn] == 1.f && pair.o.parameters[arpIsSequencer] == 1.f);
    const auto source_on = pair.o.sequencer.on;
    const auto source_off = pair.o.sequencer.off;
    auto emit = [&](int note, int velocity) {
        // Invalid notes are ignored by the original allocator as well.
        if (note < 0 || note >= 128) return;
        int32_t record[] = { (int32_t)out.frame, note, velocity };
        CHECK(std::fwrite(record, sizeof(int32_t), 3, events) == 3);
    };
    pair.o.sequencer.on = [&](int n, int v) { emit(n,v); source_on(n,v); };
    pair.o.sequencer.off = [&](int n) { emit(n,0); source_off(n); };''')
cpp=once(cpp,'CHECK(std::fclose(pcm) == 0);','CHECK(std::fclose(pcm) == 0 && std::fclose(events) == 0);')
(work/'original.cpp').write_text(cpp)

c=git_file(bench+'/shipping.c')
c=once(c,'    assert(argc == 5);','''    assert(argc == 7);
    unsigned mode = (unsigned)strtoul(argv[5], NULL, 10);
    assert(mode <= 2);
    FILE *events = mode ? fopen(argv[6], "rb") : NULL;
    struct { int32_t frame, note, velocity; } event = {-1, 0, 0};
    if (mode) {
        assert(events && fread(&event, sizeof event, 1, events) == 1);
        assert(event.frame >= 0 && event.frame < 132300);
    }''')
a=c.index('        if (at == 0) key(54, 100);')
b=c.index('        float left[128]',a)
c=c[:a]+'''        if (mode != 2) {
            if (at == 0) key(54, 100);
            if (at == 22050) key(61, 93);
            if (at == 44100) { key(54, 0); key(61, 0); key(66, 116); }
            if (at == 66150) key(66, 0);
        }
        while (event.frame == (int32_t)at) {
            assert(event.note >= 0 && event.note < 128 && event.velocity >= 0 && event.velocity < 128);
            if (mode == 2) key((unsigned)event.note, (unsigned)event.velocity);
            if (fread(&event, sizeof event, 1, events) != 1) {
                assert(feof(events) && !ferror(events));
                event.frame = -1;
            } else assert(event.frame >= (int32_t)at && event.frame < 132300);
        }
        unsigned n = 22050 - at % 22050;
        unsigned to_callback = 128 - (at % 22050) % 128;
        if (n > to_callback) n = to_callback;
        if (event.frame >= 0 && (unsigned)event.frame - at < n)
            n = (unsigned)event.frame - at;
        assert(n && n <= 128);
'''+c[b:]
c=once(c,'    assert(fclose(pcm) == 0);','''    assert(event.frame == -1);
    assert(fclose(pcm) == 0);
    if (events) assert(fclose(events) == 0);''')
(work/'shipping.c').write_text(c)

commands=json.loads((pinned/'factory-build-commands.json').read_text())
cc=next(cmd.copy() for cmd in commands if any(a.endswith('/preset_audio.cpp') for a in cmd))
for flag,file in [('-c','original.cpp'),('-o','original.o'),('-MF','original.d')]:
    cc[cc.index(flag)+1]=str(work/file)
link=commands[-1].copy()
link[link.index(str(pinned/'factory-6.o'))]=str(work/'original.o')
link[link.index('-o')+1]=str(work/'original')
flags=['-std=c11','-O2','-fno-fast-math','-ffp-contract=off','-ffunction-sections','-fdata-sections',
       '-DX0X_HOST','-DX0X_SYNTHONE_PATCH=1','-DX0X_ORIGINAL_SYNTH=1','-DX0X_WOBBLE=0','-DX0X_DEV_WOBBLE=0',
       '-DORIGINAL_USE_SOFT_FMA','-DMIAMI_FX_NO_HOST_FENV','-DPROJECT_CRUSH_LFO=1','-DS1_PROJ_SUB=1',
       '-Iexperiments/original_synth','-Iexperiments/wobble','-Ifirmware/src','-Ifirmware/src/app',
       '-Ifirmware/src/dsp','-Ibuild/gen']
inputs=[*replay.ORIGINAL_SYNTH_UNITS,*replay.ORIGINAL_SYNTH_BACKENDS]
# Verify every compiled firmware source still matches the stated preimage.
for name in inputs:
    if not name.startswith('firmware/'): continue
    before=subprocess.check_output(['git','-C',root,'show',source+':'+name])
    assert before==(root/name).read_bytes(),name
candidate=['cc',*flags,str(work/'shipping.c'),*inputs,'-Wl,--gc-sections','-lm','-o',str(work/'shipping')]
(work/'commands.json').write_text(json.dumps(dict(cwd=str(root), compile_original=cc, link_original=link, compile_candidate=candidate),indent=2)+'\n')
for cmd in (cc,link,candidate): subprocess.run(cmd,check=True)

indices=[139,664,482,410,138,10,221,213,481]
baseline_rows = root / "evidence/sound-resume/synthone-pitch-lfo/factory-40.jsonl.gz"
with gzip.open(baseline_rows, "rt") as records:
    expected = {row["index"]: row for row in map(json.loads, records)}
result=[]
for i in indices:
    prefix=work/str(i)
    prefix.mkdir(exist_ok=True)
    oracle, trace=prefix/'original.f32',prefix/'events.i32'
    original_cmd=[work/'original',pinned/'factory-inputs/public-morphs.f32',pinned/'factory-inputs/compact.bin',
                  pinned/'commands/preset-commands.bin',str(i),oracle,trace]
    subprocess.run(original_cmd,check=True)
    reference=np.fromfile(oracle,dtype='<f4').reshape(-1,2)
    baseline=expected[i]
    assert digest(oracle)==baseline['original_sha256'], i
    events=list(struct.iter_unpack('<iii',trace.read_bytes()))
    assert events and events==sorted(events,key=lambda e:e[0])
    row=dict(index=i,name=baseline['name'],original_sha256=digest(oracle),
             event_sha256=digest(trace),events=events,event_count=len(events),note_ons=sum(v>0 for _,_,v in events),
             first_note_on=min(t for t,_,v in events if v>0),passes_before=baseline['pass'], modes={})
    for mode,name in [(0,'direct'),(1,'split_control'),(2,'oracle_events')]:
        f,k=prefix/(name+'.f32'),prefix/(name+'.u8')
        cmd=[work/'shipping',targets,str(i),f,k,str(mode),trace]
        subprocess.run(cmd,check=True)
        rs=np.fromfile(f,dtype='<f4').reshape(-1,2)
        assert np.isfinite(rs).all() and list(k.read_bytes())==baseline['panel']
        if not mode: assert digest(f)==baseline['stock_sha256'],i
        scores=[replay.score(reference[:,ch],rs[:,ch]) for ch in (0,1)]
        row['modes'][name]=dict(sha256=digest(f),channels=scores,passed=all(x['pass'] for x in scores))
    assert row["modes"]["direct"]["sha256"] == row["modes"]["split_control"]["sha256"]
    result.append(row)
    print(json.dumps({k:v for k,v in row.items() if k!='events'}),flush=True)
(work/'focused-results.json').write_text(json.dumps(dict(preimage=source,rows=result),indent=2)+'\n')
