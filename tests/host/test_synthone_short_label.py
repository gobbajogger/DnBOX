#!/usr/bin/env python3
"""Compile actual short-name function; cover single and paired Synth One slots."""
from pathlib import Path
import os, subprocess, tempfile, re
root=Path(__file__).resolve().parents[2]
source=(root/'firmware/src/app/ui.c').read_text()
match=re.search(r'static const char \*part_s\(int i\)\n\{.*?\n\}',source,re.S)
assert match
fixture='''#include <assert.h>
#include <string.h>
#define X0X_BASS_PRESET_PREVIEW 1
#define X0X_BASS_TAGGED_SOUND 0
#define PART_303A 2
#define PART_303B 3
#define PART_BRK 4
#define ORIGINAL_SOUND_JUNO 2
#define ORIGINAL_SOUND_SYNTHONE 3
static const char *PART_N[]={"909","808","A","B","BRK"};
static int banks[2];
static int part_is_303(int i){return i==2||i==3;}
static int engine_bass_bank(int v){return banks[v];}
'''+match[0]+'''
int main(void){
 for(int v=0;v<2;v++){
  banks[v]=3;banks[1-v]=0;
  assert(!strcmp(part_s(v+2),"S1"));
  banks[1-v]=3;
  assert(!strcmp(part_s(v+2),v?"S1B":"S1A"));
 }
 assert(!strcmp(part_s(0),"909"));assert(!strcmp(part_s(1),"808"));assert(!strcmp(part_s(4),"BRK"));
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(fixture)
 subprocess.run([os.environ.get('CC','cc'),'-std=c99','-Wall','-Wextra','-Werror',str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS Synth One short labels: single/paired A/B and non-synth identities')
