#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Capture one actual source tree into first-slice evaluator-compatible receipts.
Never edits input, invents historical provenance, or implies whole-UI qualification.
"""
import argparse, json, shutil, subprocess
from pathlib import Path
from run import closure, run_one, write

IGNORED = ('.git', 'build', '__pycache__')

def git_fact(source, *args):
    p = subprocess.run(['git', '-C', str(source), *args], text=True,
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return p.stdout.strip() if p.returncode == 0 else None

def stage_source(source, output, expected_head=None):
    source = source.resolve(); output = output.resolve()
    if output == source or source in output.parents:
        raise ValueError('output must be outside source')
    head = git_fact(source, 'rev-parse', 'HEAD')
    if expected_head is not None and head != expected_head:
        raise ValueError('source HEAD differs from requested exact full HEAD')
    # Whole working-tree closure, not an assertion that HEAD contains these bytes.
    before = closure(source)
    output.mkdir(parents=True, exist_ok=False)
    write(output/'input-source-closure.json', before)
    work = output/'captured-source'
    shutil.copytree(source, work, symlinks=True,
                    ignore=shutil.ignore_patterns(*IGNORED))
    if closure(work) != before or closure(source) != before:
        raise ValueError('source changed during capture or copied bytes differ')
    profile = {'schema':'dnbox-measured-source-v1', 'sourcePath':str(source),
               'head':head, 'gitStatus':git_fact(source, 'status', '--porcelain'),
               'inputClosureSha256':before['sha256'],
               'scope':'Actual captured working-tree bytes, not HEAD-only qualification.',
               'hardware':'NOT_RUN'}
    old = work/'SOURCE-PROFILE.json'
    if old.exists():
        # Preserve original profile bytes in source manifest and separate artifact.
        shutil.copy2(old, output/'input-SOURCE-PROFILE.json')
    write(old, profile)
    write(output/'capture.json', profile)
    return work

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--expected-head',help='Exact full local HEAD; working changes remain recorded')
    a=p.parse_args()
    work=stage_source(a.source,a.output,a.expected_head)
    report=run_one('render',work,a.output.resolve())
    write(a.output/'capture-result.json',report)
    print(json.dumps({'renderDir':str(a.output.resolve()/'render'),
                     'sourceClosureSha256':report.get('sourceClosureSha256'),
                     'detectorExit':report.get('detectorExit'),
                     'coverageComplete':False}))
    return report.get('detectorExit',2)

if __name__=='__main__':raise SystemExit(main())
