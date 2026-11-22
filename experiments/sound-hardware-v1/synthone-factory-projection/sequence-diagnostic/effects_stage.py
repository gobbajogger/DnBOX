#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Hash-verified original FX substitution on the nine retained voice traces."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

import numpy as np

def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def substitute(text, old, new):
    assert text.count(old) == 1, old
    return text.replace(old, new)

def main():
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument('repo', type=Path)
    cli.add_argument('resumed', type=Path)
    cli.add_argument('output', type=Path)
    args = cli.parse_args()
    repo, resumed, out = (p.resolve() for p in (args.repo,args.resumed,args.output))
    out.mkdir(exist_ok=True)
    factory, original_runs, dry = (resumed / p for p in ('factory','nine','dry'))
    fixture = factory / 'factory-inputs'
    target = resumed/'panel/panel-targets.f32'
    commands = json.loads((original_runs/'commands.json').read_text())
    expected = json.loads((original_runs/'focused-results.json').read_text())
    expected_dry = {r['index']:r for r in json.loads((dry/'result.json').read_text())['rows']}
    spec = importlib.util.spec_from_file_location('project_score',
        repo/'experiments/sound-hardware-v1/synthone-factory-projection/replay.py')
    scorer = importlib.util.module_from_spec(spec)
    # The shared scorer discovers its local build module using cwd.
    import os
    os.chdir(repo)
    spec.loader.exec_module(scorer)
    source_header = fixture/'tests/fixtures/synthone-factory/fullfx/source_fx.hpp'
    header = source_header.read_text()
    header = substitute(header, ' void frame(float input,',
        ' unsigned omit{};\n void frame(float input,')
    header = substitute(header, '        // REVERB INPUT HIPASS FILTER',
        '''        if (omit == 1u) { mixedDelayL = phaserOutL; mixedDelayR = phaserOutR; }
        // REVERB INPUT HIPASS FILTER''')
    header = substitute(header, '        // MASTER COMPRESSOR/LIMITER',
        '''        if (omit == 2u) { reverbCrossfadeOutL = mixedDelayL; reverbCrossfadeOutR = mixedDelayR; }
        // MASTER COMPRESSOR/LIMITER''')
    header = substitute(header, '        // crossfade wet reverb with wet+dry delay',
        '''        if (omit == 3u) { wetReverbLimiterL = 0.f; wetReverbLimiterR = 0.f; }
        // crossfade wet reverb with wet+dry delay''')
    (out/'fx_probe.hpp').write_text(header)
    source = (dry/'original.cpp').read_text()
    source = substitute(source,'#include "source_fx.hpp"','#include "fx_probe.hpp"')
    source = substitute(source,'    FILE *dry{};','    FILE *dry{}, *injected{};')
    source = substitute(source,'    out.fx.frame(out.left[frame],',
        '''    float excitation = out.left[frame];
    if (out.injected) CHECK(std::fread(&excitation,sizeof(float),1,out.injected)==1);
    out.fx.frame(excitation,''')
    source = substitute(source,'CHECK(argc == 8);','CHECK(argc == 10);')
    source = substitute(source,'    Output out;','''    Output out;
    out.fx.omit=(unsigned)std::stoul(argv[8]);
    CHECK(out.fx.omit <= 3);
    if (std::strcmp(argv[9],"-")) {
        out.injected=std::fopen(argv[9],"rb");
        CHECK(out.injected);
    }''')
    source = substitute(source,'    CHECK(std::fclose(pcm)',
        '''    if (out.injected) CHECK(std::fgetc(out.injected)==EOF && !std::ferror(out.injected) && std::fclose(out.injected)==0);
    CHECK(std::fclose(pcm)''')
    (out/'original.cpp').write_text(source)
    executed = {}
    for key in ('compile_original','link_original'):
        command=[str(out/Path(arg).name) if Path(arg).parent == original_runs else arg for arg in commands[key]]
        subprocess.run(command,cwd=repo,check=True)
        executed[key] = command
    (out/'commands.json').write_text(json.dumps(executed,indent=2)+'\n')

    def score(ref, path):
        pcm=np.fromfile(path,dtype='<f4').reshape(-1,2)
        assert len(pcm)==132300 and np.isfinite(pcm).all()
        return [scorer.score(ref[:,ch],pcm[:,ch]) for ch in (0,1)]

    def oracle(index, output, events, source_dry, omit, injection='-'):
        subprocess.run([out/'original',fixture/'public-morphs.f32',fixture/'compact.bin',
                        factory/'commands/preset-commands.bin',str(index),output,events,source_dry,
                        str(omit),injection],check=True)

    rows=[]
    for row in expected['rows']:
        index = row['index']
        case = out / str(index)
        case.mkdir()
        source_pcm, events, source_dry = (case/p for p in ('original.f32','events.i32','original-dry.f32'))
        oracle(index,source_pcm,events,source_dry,0)
        assert digest(source_pcm)==row['original_sha256']
        assert digest(events)==row['event_sha256']
        assert digest(source_dry)==expected_dry[index]['original_dry_sha256']
        ref=np.fromfile(source_pcm,dtype='<f4').reshape(-1,2)
        result={'index':index,'name':row['name'],'event_sha256':digest(events),
                'original_sha256':digest(source_pcm),'arms':{}}
        inject_events, dry_compare = case/'replay-events.i32', case/'replay-dry.f32'
        for mode,name in [(0,'direct'),(2,'oracle_events')]:
            firmware, panel, firmware_dry = (case/(name+p) for p in ('.f32','.u8','-dry.f32'))
            subprocess.run([dry/'shipping',target,str(index),firmware,panel,str(mode),events,firmware_dry],check=True)
            assert digest(firmware)==row['modes'][name]['sha256']
            assert panel.read_bytes()==(original_runs/str(index)/'direct.u8').read_bytes()
            if mode == 2:
                assert digest(firmware_dry)==expected_dry[index]['firmware_dry_sha256']
            variants={'firmware':{'sha256':digest(firmware),'scores':score(ref,firmware)}}
            for omit,label in enumerate(('original_all_fx','original_without_delay','original_without_reverb','original_muted_wet_reverb')):
                path=case/(name+'-'+label+'.f32')
                oracle(index,path,inject_events,dry_compare,omit,firmware_dry)
                assert digest(inject_events)==row['event_sha256'] and digest(dry_compare)==digest(source_dry)
                variants[label]={'sha256':digest(path),'scores':score(ref,path)}
            result['arms'][name]=variants
        rows.append(result)
        print(index,flush=True)
    bundle={'source':'f6c2f17087dea9de52dc77f88dd271aab6692ef1',
            'source_fx_header_sha256':digest(source_header),'rows':rows}
    (out/'effects-stage-result.json').write_text(json.dumps(bundle,indent=2)+'\n')

if __name__=='__main__':
    main()
