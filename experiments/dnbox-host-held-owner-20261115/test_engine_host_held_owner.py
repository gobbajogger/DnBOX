#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compile the exact host observer with the real runtime storage declaration."""
import argparse, os, shlex, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
def observer():
    source=(ROOT/'firmware/src/app/engine.c').read_text()
    start=source.index('int engine_bass_keys_held(void)')
    brace=source.index('{',start);depth=1;end=brace+1
    while depth:
        depth += (source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]
HARNESS=r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "original_runtime.h"
#define NBASS 2
static original_runtime_part original_bass[NBASS];
static uint8_t remote[NBASS][128];
static unsigned checks;
#define CHECK(x) do { ++checks; if(!(x)){fprintf(stderr,"FAIL %u line%d: %s\n",checks,__LINE__,#x);return 1;} }while(0)
@@OBSERVER@@
static void clear_rows(int bound)
{
    memset(original_bass,0xa5,sizeof original_bass);
    memset(remote,0,sizeof remote);
    for(unsigned v=0;v<NBASS;++v){
        memset(original_bass[v].keys,0,sizeof original_bass[v].keys);
        original_bass[v].remote_keys=bound?remote[v]:0;
    }
}
int main(void)
{
    clear_rows(0);CHECK(engine_bass_keys_held()==0);
    clear_rows(1);CHECK(engine_bass_keys_held()==0);
    for(unsigned v=0;v<NBASS;++v) for(unsigned input=0;input<ORIGINAL_INPUT_COUNT;++input)
      for(unsigned key=0;key<128;++key){
        clear_rows(1);
        uint8_t *row=input==ORIGINAL_INPUT_REMOTE?remote[v]:original_bass[v].keys[input];
        unsigned other=v^1u;
        uint8_t *other_row=input==ORIGINAL_INPUT_REMOTE?remote[other]:original_bass[other].keys[input];
        row[key]=1;CHECK(engine_bass_keys_held()==(1<<v));
        other_row[key]=127;CHECK(engine_bass_keys_held()==3);
        row[key]=0;CHECK(engine_bass_keys_held()==(1<<other));
        other_row[key]=0;CHECK(engine_bass_keys_held()==0);
      }
    clear_rows(0);
    for(unsigned input=0;input<ORIGINAL_INPUT_REMOTE;++input){original_bass[0].keys[input][input]=1;CHECK(engine_bass_keys_held()==1);}
    for(unsigned input=0;input<ORIGINAL_INPUT_REMOTE;++input){original_bass[0].keys[input][input]=0;CHECK(engine_bass_keys_held()==(input+1<ORIGINAL_INPUT_REMOTE?1:0));}
    printf("Exact host held-owner observer: %u checks PASS\n",checks);return 0;
}
'''
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--mode',choices=['all','normal','ubsan'],default='all');args=ap.parse_args()
    for mode in (['normal','ubsan'] if args.mode=='all' else [args.mode]):
      with tempfile.TemporaryDirectory(prefix='dnbox-host-held-owner-') as td:
        td=Path(td);c=td/'test.c';exe=td/'test';c.write_text(HARNESS.replace('@@OBSERVER@@',observer()))
        flags=[] if mode=='normal' else ['-fsanitize=undefined,bounds,float-cast-overflow','-fno-sanitize-recover=all']
        cmd=shlex.split(os.environ.get('CC','cc'))+['-std=c11','-O2','-Wall','-Wextra','-Werror']+flags+['-I'+str(ROOT/'firmware/src/app'),'-I'+str(ROOT/'firmware/src'),'-I'+str(ROOT/'experiments/original_synth'),'-I'+str(ROOT/'experiments/wobble'),str(c),'-o',str(exe)]
        subprocess.run(cmd,check=True);subprocess.run([str(exe)],check=True)
if __name__=='__main__':main()
