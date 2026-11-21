#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Cap metrics must match half-intensity production pixels, for every font role."""
import argparse
import contextlib
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile


def run(source, output):
    source=source.resolve();output=output.resolve();output.mkdir(parents=True,exist_ok=True)
    generator=source/'tools/gen_font.py';gfx=source/'firmware/src/gfx.c'
    fixture=Path(__file__).with_name('host')/'font_cap_metrics_test.c'
    spec=importlib.util.spec_from_file_location('cap_test_generator',generator)
    gen=importlib.util.module_from_spec(spec);spec.loader.exec_module(gen)
    cc=shlex.split(os.environ.get('CC','cc'));results=[];hashes={}
    env=dict(os.environ);env['ASAN_OPTIONS']=env.get('ASAN_OPTIONS','')+':detect_leaks=0'
    for family in ('barlow','terminus'):
        gen.FONTSET=family;gen.SIZES=gen.FONTSETS[family]
        for storage,compressed,codec in [('raw',False,None),('packed',True,None),('packbits',True,'packbits')]:
            out=output/(family+'-'+storage);out.mkdir(exist_ok=True)
            capture=io.StringIO()
            with contextlib.redirect_stdout(capture):gen.main(out/'felucca_font.h',compressed=compressed,codec=codec)
            (out/'generate.log').write_text(capture.getvalue())
            command=cc+['-std=c11','-O1','-g','-Wall','-Wextra','-Wno-unused-function','-I'+str(out),
                        '-DGFX_SOURCE="'+str(gfx)+'"',str(fixture)]
            for sanitized in (False,True):
                name='sanitized' if sanitized else 'normal';exe=out/name
                flags=['-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer'] if sanitized else []
                compile_run=subprocess.run(command+flags+['-o',str(exe)],capture_output=True,text=True)
                (out/(name+'-compile.log')).write_text(compile_run.stdout+compile_run.stderr)
                if compile_run.returncode:raise RuntimeError('compile failed: '+str(out/(name+'-compile.log')))
                positive=subprocess.run([str(exe)],env=env,capture_output=True,text=True)
                (out/(name+'.log')).write_text(positive.stdout+positive.stderr)
                if positive.returncode or len(re.findall(r'^CAP_FACE ',positive.stdout,re.M))!=6:
                    raise RuntimeError('positive failed: '+str(out/(name+'.log')))
                digest=re.search(r'^PASS rendered cap metadata faces=6 raster_hash=([0-9a-f]+)$',positive.stdout,re.M)
                if not digest:raise RuntimeError('missing terminal cap result')
                previous=hashes.setdefault(family,digest[1])
                if previous!=digest[1]:raise RuntimeError('raw/packed production raster differs: '+str(out))
                row={'family':family,'storage':storage,'sanitized':sanitized,'exit':positive.returncode,
                     'rasterHash':digest[1],'faces':6,'log':str(out/(name+'.log'))}
                if not sanitized:
                    row['negativeControls']={}
                    for mutation in ('top','bottom'):
                        neg=subprocess.run([str(exe),'--negative-'+mutation],env=env,capture_output=True,text=True)
                        (out/('negative-'+mutation+'.log')).write_text(neg.stdout+neg.stderr)
                        if neg.returncode!=1 or 'CAP_METRIC_MISMATCH' not in neg.stderr:
                            raise RuntimeError('metric mutant did not fail at intended check: '+str(out))
                        row['negativeControls'][mutation]={'exit':neg.returncode,'marker':'CAP_METRIC_MISMATCH'}
                results.append(row)
    report={'status':'PASS','source':str(source),'sourceHashes':{str(p.relative_to(source)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (generator,gfx)},
            'cases':results,'faceChecks':72,'normalMetricMutants':12,'sanitizers':'AddressSanitizer and UndefinedBehaviorSanitizer; leak checking disabled for ptrace environment',
            'scope':'Final white-on-black RGB565 pixels from production painter, half-intensity threshold (equivalent alpha8); all6 roles in2 font families, raw/native packed/PackBits; declared glyph ranges include retained width-zero glyphs and clipped coordinates. No device or layout acceptance.'}
    (output/'result.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report))


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--source',type=Path,default=Path(__file__).resolve().parents[1]);parser.add_argument('--output',type=Path)
    args=parser.parse_args()
    if args.output:run(args.source,args.output)
    else:
        with tempfile.TemporaryDirectory(prefix='font-cap-metrics-') as directory:run(args.source,Path(directory))
