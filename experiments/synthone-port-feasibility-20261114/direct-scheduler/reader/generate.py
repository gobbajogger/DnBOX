#!/usr/bin/env python3
"""Derive only the admitted runtime morph-load and voice-call changes."""
from pathlib import Path
import argparse,hashlib,json
VOICE='probe/portable_voice.cpp'
MORPH='audiokit-4.9.2/AudioKit/Core/SoundpipeExtension/modules/oscmorph2d.c'
PINS={VOICE:'987f18732cedf60455dd5f89cbde521a7fd7ecea5493f3e463d013f2b69b1b0c',MORPH:'455c56d1f12e3b62d89eea1f83ef9bbd14402b37b14db4b54051383a1d64af0c'}
def digest(b):return hashlib.sha256(b).hexdigest()
def section(source,start):
 assert source.count(start)==1,start
 begin=source.index(start);brace=source.index('{',begin);depth=0
 for end in range(brace,len(source)):
  if source[end]=='{':depth+=1
  if source[end]=='}':
   depth-=1
   if not depth:return source[begin:end+1],source[brace+1:end]
 raise ValueError('missing function end')
def replace(s,a,b):
 assert s.count(a)==1,repr(a)
 return s.replace(a,b)
def main():
 p=argparse.ArgumentParser();p.add_argument('--experiment-root',type=Path,required=True);p.add_argument('--output-dir',type=Path,required=True);a=p.parse_args();sources={}
 for path,sha in PINS.items():
  raw=(a.experiment_root/path).read_bytes();assert digest(raw)==sha,'original source identity mismatch: '+path;sources[path]=raw.decode()
 voice=sources[VOICE];morph=sources[MORPH]
 original_run,run=section(voice,'void S1NoteState::run(')
 _,get_param=section(voice,'inline float S1NoteState::getParam(')
 _,sample_rate=section(voice,'inline int S1NoteState::sampleRate(')
 nn,_=section(voice,'static inline float nnToHz(')
 old='sp_oscmorph2d_compute';assert run.count(old)==2
 adapted_run=run.replace(old,'runtime_morph_compute')
 assert adapted_run.replace('runtime_morph_compute',old)==run
 refs=['kernel','oscmorph1','oscmorph2','subOsc','fmOsc','noise','morphCrossFade','filterCrossFade','loPass','bandPass','hiPass','adsr','fadsr','internalGate','amp','filter']
 cpp='// Generated from pinned original source. See DERIVATION.json and notices.\n#include "runtime_note.hpp"\n#include "morph_compute.h"\nnamespace {\n'+nn+'\n}\nnamespace s1scheduler_reader {\nbool run_note(S1NoteState &note,int frameIndex,float *outL,float *outR,const s1_morph_binding &binding){\n'
 cpp+=''.join('    auto &'+name+'=note.'+name+';\n'for name in refs)
 cpp+='    auto getParam=[&](S1Parameter param)->float {'+get_param+'};\n'
 cpp+='    auto sampleRate=[&]()->int {'+sample_rate+'};\n'
 cpp+='    bool reader_ok=true;\n    auto runtime_morph_compute=[&](sp_data *sp,sp_oscmorph2d *osc,SPFLOAT *in,SPFLOAT *out)->int {\n        const int rc=s1_runtime_morph_compute(sp,osc,in,out,&binding);\n        if(rc!=SP_OK)reader_ok=false;\n        return rc;\n    };\n'
 cpp+=adapted_run+'\n    return reader_ok;\n}\n}\n'
 original_morph,_=section(morph,'int sp_oscmorph2d_compute(')
 c=replace(original_morph,'int sp_oscmorph2d_compute(sp_data *sp, sp_oscmorph2d *osc, SPFLOAT *in, SPFLOAT *out)','int s1_runtime_morph_compute(sp_data *sp, sp_oscmorph2d *osc, SPFLOAT *in, SPFLOAT *out, const s1_morph_binding *binding)')
 c=replace(c,'{\n    sp_ftbl *ftp1;','{\n    if(!sp||!osc||!out||!s1_morph_binding_valid(binding)||!osc->tbl||!osc->tbl[0]||!osc->fbl||osc->nft!=4||osc->nbl!=13||!isfinite(osc->freq)||osc->freq<0.f||!isfinite(osc->amp)||osc->lphs<0||(uint32_t)osc->lphs>SP_FT_PHMASK) return SP_NOT_OK;\n    sp_ftbl *ftp1;')
 c=replace(c,'    SPFLOAT *ft1, *ft2;','    int row1, row2;')
 c=replace(c,'    /* Use only the fractional part of the position or 1 */','    if(bandlimitIndex<0||bandlimitIndex>=13||!isfinite(osc->wtpos)||osc->wtpos<0.f||osc->wtpos>=2147483648.f) return SP_NOT_OK;\n    /* Use only the fractional part of the position or 1 */')
 c=replace(c,'    const SPFLOAT wtfrac = findex - index;','    const SPFLOAT wtfrac = findex - index;\n    if(index<0||index>=52||!osc->tbl[index]||osc->tbl[index]->size!=4096||osc->tbl[index]->lobits>=31) return SP_NOT_OK;')
 c=replace(c,'    ft1 = osc->tbl[index]->tbl;','    row1 = index;')
 c=replace(c,'        ft2 = ft1;','        row2 = row1;')
 c=replace(c,'        ft2 = osc->tbl[index + 1]->tbl;','        row2 = index + 1;')
 reads='''    uint32_t w11,w21,w12,w22;
    SPFLOAT f11,f21,f12,f22;
    if(!s1_morph_read_word(binding,(unsigned)row1,(unsigned)pos,&w11)||
       !s1_morph_read_word(binding,(unsigned)row2,(unsigned)pos,&w21)||
       !s1_morph_read_word(binding,(unsigned)row1,(unsigned)((pos+1)%ftp1->size),&w12)||
       !s1_morph_read_word(binding,(unsigned)row2,(unsigned)((pos+1)%ftp1->size),&w22)) return SP_NOT_OK;
    memcpy(&f11,&w11,4);memcpy(&f21,&w21,4);memcpy(&f12,&w12,4);memcpy(&f22,&w22,4);
'''
 c=replace(c,'    v1 = (1 - wtfrac) * ',reads+'\n    v1 = (1 - wtfrac) * ')
 changes=[('*(ft1 + pos)','f11'),('*(ft2 + pos)','f21'),('*(ft1 + ((pos + 1) % ftp1->size))','f12'),('*(ft2 + ((pos + 1) % ftp1->size))','f22')]
 for old,new in changes:c=replace(c,old,new)
 c='// Generated morph-only direct-read derivative. See DERIVATION.json.\n#include "morph_compute.h"\n#include <math.h>\n#include <string.h>\n'+c+'\n'
 out=a.output_dir;out.mkdir(parents=True,exist_ok=True)
 outputs={'runtime_note.cpp':cpp,'morph_compute.c':c}
 manifest={'schema':1,'scope':'Runtime-only voice call adapter and four morph loads; original oracle/member/C morph unchanged','inputs':[{'path':p,'sha256':h}for p,h in PINS.items()],'original_member_function_sha256':digest(original_run.encode()),'original_member_body_sha256':digest(run.encode()),'original_morph_function_sha256':digest(original_morph.encode()),'private_helper_bodies_sha256':{'getParam':digest(get_param.encode()),'sampleRate':digest(sample_rate.encode()),'nnToHz':digest(nn.encode())},'voice_allowed_changes':['sixteen references to actual note fields','exact private helper bodies','two morph call-name redirects','explicit failed-read result propagation'],'morph_allowed_changes':['distinct symbol and binding parameter','admitted metadata/bounds rejection','row-index bookkeeping instead of raw payload pointers','four checked direct-word reads and bit copies','unchanged original interpolation/phase arithmetic'],'outputs':[{'path':p,'bytes':len(v.encode()),'sha256':digest(v.encode())}for p,v in outputs.items()]}
 for name,value in outputs.items():(out/name).write_text(value)
 (out/'DERIVATION.json').write_text(json.dumps(manifest,indent=2)+'\n')
 print(json.dumps({'outputs':manifest['outputs'],'original_body_sha256':manifest['original_member_body_sha256']}))
if __name__=='__main__':main()
