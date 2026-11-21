#!/usr/bin/env python3
"""Create a real, disposable Juno-slot replacement resource fixture.

All edited files live in this review directory. Neither the frozen candidate
nor the exact baseline changes. This is an 18-control stock-slot bridge, not
an implementation of SuperJ6's 218 original control bindings or Synth One.
"""
from pathlib import Path
import hashlib
import json
import shutil

HERE = Path(__file__).resolve().parent
BASE = HERE.parent / "native-baseline-76040"
TREE = HERE / "replacement-tree"
if TREE.exists():
    raise SystemExit("Refusing to overwrite an existing replacement fixture")
shutil.copytree(BASE, TREE)
DSP = TREE / "firmware/src/dsp"
for name in ["dnbx_core.c", "dnbx_core.h"]:
    shutil.copyfile(HERE / "src" / name, DSP / name)

header = '''/* SPDX-License-Identifier: GPL-3.0-only */
/* Isolated RESOURCE FIXTURE: stock 18-control facade over the fresh core.
 * This file is never a production proposal or original-control equivalence. */
#pragma once
#include <stdint.h>
#include "envelope_snapshot.h"
#include "dnbx_core.h"
#define JUNO6_VOICES 6u
#define JUNO6_CHORUS_BYTES 512u
#define JUNO6_CHORUS_SAMPLES 256u
typedef struct {
 dnbx_part core;
 float macro[18];
 float hpf, hp_state[2], hp_input[2];
} juno6_t;
typedef struct {
 int16_t delay[JUNO6_CHORUS_SAMPLES];
 uint16_t write;
 uint32_t phase;
 float sample_rate,depth_ms,rate_hz,mix;
} juno6_chorus_t;
void juno6_init(juno6_t*,float);
int juno6_note_on(juno6_t*,uint8_t,uint8_t);
void juno6_note_off(juno6_t*,uint8_t);
void juno6_all_off(juno6_t*);
int juno6_render_stereo(juno6_t*,float*,float*,unsigned);
void juno6_chorus_init(juno6_chorus_t*,float);
void juno6_chorus_set(juno6_chorus_t*,float,float,float);
void juno6_chorus_process(juno6_chorus_t*,float*,float*,unsigned);
int juno6_set(juno6_t*,unsigned,float);
int juno6_active(const juno6_t*);
int juno6_read_envelope(const juno6_t*,juno6_envelope_snapshot*);
'''
(DSP / "juno6.h").write_text(header)

old_source = (BASE / "firmware/src/dsp/juno6.c").read_text()
chorus = old_source[old_source.index("void juno6_chorus_init"):old_source.index("\nint juno6_active")]
adapter = '''/* SPDX-License-Identifier: GPL-3.0-only */
/* RESOURCE FIXTURE ONLY. Existing stock18 controls drive both candidate layers.
 * Exact stock chorus code is appended unchanged. No original engine code.
 * This does not claim sound equivalence or the original218-control semantics. */
#include "juno6.h"
#include "fastmath.h"
#include <string.h>
#define TWO_PI 6.28318530717958647692f
static float clampf(float x,float a,float b){return x<a?a:x>b?b:x;}

void juno6_init(juno6_t *s,float sr)
{
 memset(s,0,sizeof *s);
 (void)dnbx_init(&s->core,sr>8000?sr:44100);
 s->macro[0]=.35f;s->macro[4]=.35f;s->macro[10]=.12f;
 s->macro[15]=1;s->macro[16]=.5f;
}
int juno6_note_on(juno6_t *s,uint8_t note,uint8_t velocity)
{ return dnbx_note_on(&s->core,note,velocity)>=0; }
void juno6_note_off(juno6_t *s,uint8_t note)
{ (void)dnbx_note_off(&s->core,note); }
void juno6_all_off(juno6_t *s)
{
 (void)dnbx_all_off(&s->core);s->macro[17]=0;
 memset(s->hp_state,0,sizeof s->hp_state);memset(s->hp_input,0,sizeof s->hp_input);
}
int juno6_active(const juno6_t *s)
{ return dnbx_active_voices(&s->core)>0; }

int juno6_set(juno6_t *s,unsigned id,float x)
{
 if(!s||id>=18||!(x>=0&&x<=1))return 0;
 s->macro[id]=x;
 if(id==17)return dnbx_set_sustain(&s->core,x>=.5f)==DNBX_OK;
 if(id==9){s->hpf=x;return 1;}
 if(id==13||id==14)return 1; /* Existing runtime owns stock chorus parameters. */
 for(unsigned l=0;l<DNBX_LAYERS;l++){
  dnbx_layer_params p;
  if(dnbx_get_layer(&s->core,l,&p)!=DNBX_OK)return 0;
  switch(id){
  case 0:p.waveform=(2+x)/3;break;
  case 1:p.sub_gain=x*.5f;break;
  case 2:p.cutoff_hz=35+x*x*12000;break;
  case 3:p.resonance=x;break;
  case 4:p.filter_env_hz=7000*x;break;
  case 5:p.amp.attack_s=p.filter.attack_s=.002f+x*x*3;break;
  case 6:p.amp.decay_s=p.filter.decay_s=.01f+x*x*3;break;
  case 7:p.amp.sustain=p.filter.sustain=x;break;
  case 8:p.amp.release_s=p.filter.release_s=.01f+x*x*4;break;
  case 10:p.lfo_hz=.1f+8*x*x;break;
  case 11:p.lfo_pwm_depth=x*.2f;break;
  case 12:p.pulse_width=clampf(.1f+x*.8f,.03f,.5f);break;
  case 15:p.gain=x;break;
  case 16:p.tune_semitones=(x-.5f)*4;break;
  default:break;
  }
  if(dnbx_set_layer(&s->core,l,&p)!=DNBX_OK)return 0;
 }
 return 1;
}

int juno6_render_stereo(juno6_t *s,float *left,float *right,unsigned frames)
{
 if(dnbx_render(&s->core,left,right,frames,0)!=DNBX_OK)return 0;
 if(s->hpf>0){
  const float hp=1/(1+TWO_PI*(15+2000*s->hpf*s->hpf)/s->core.sample_rate);
  for(unsigned i=0;i<frames;i++){
   float y=hp*(s->hp_state[0]+left[i]-s->hp_input[0]);
   s->hp_input[0]=left[i];s->hp_state[0]=y;left[i]=y;
   y=hp*(s->hp_state[1]+right[i]-s->hp_input[1]);
   s->hp_input[1]=right[i];s->hp_state[1]=y;right[i]=y;
  }
 }
 return 1;
}

static unsigned legacy_stage(unsigned stage)
{
 if(stage==DNBX_ENV_IDLE)return 0;
 if(stage==DNBX_ENV_ATTACK)return 1;
 if(stage==DNBX_ENV_RELEASE_HOLD||stage==DNBX_ENV_RELEASE)return 4;
 return stage==DNBX_ENV_SUSTAIN?3u:2u;
}
int juno6_read_envelope(const juno6_t *s,juno6_envelope_snapshot *out)
{
 if(!s||!out)return 0;
 juno6_envelope_snapshot value={0};
 const dnbx_envelope_params *p=&s->core.layer[0].params.amp;
 value.adsr[0]=p->attack_s;value.adsr[1]=p->decay_s;
 value.adsr[2]=p->sustain;value.adsr[3]=p->release_s;
 value.selected_voice=-1;value.shared_amp_filter=0;
 for(unsigned i=0;i<DNBX_VOICES;i++)for(unsigned l=0;l<DNBX_LAYERS;l++){
  const dnbx_voice *v=&s->core.voice[i];
  const dnbx_envelope_state *e=&v->layer[l].amp;
  if(e->stage&&(value.selected_voice<0||e->level>value.level)){
   value.selected_voice=(int)i;value.level=e->level;
   value.stage=legacy_stage(e->stage);value.gate=v->gate;value.active=1;
  }
 }
 *out=value;return 1;
}

'''
(DSP / "juno6.c").write_text(adapter + chorus)

runtime = TREE / "firmware/src/app/original_runtime.c"
text = runtime.read_text()
old_view = '''        for(unsigned i=0;i<JUNO6_VOICES;i++){
            const juno6_voice_t *v=&p->dsp.juno.synth.voice[i];
            if(v->stage && v->envelope>*level){*level=v->envelope;*state=v->stage==1?0u:v->stage==4?3u:1u;*key_down=v->gate;}
        }'''
new_view = '''        juno6_envelope_snapshot view;
        if(juno6_read_envelope(&p->dsp.juno.synth,&view)&&view.active){
            *level=view.level;*state=view.stage==1?0u:view.stage==4?3u:1u;*key_down=view.gate;
        }'''
old_render = '''        juno6_render(&p->dsp.juno.synth,left,frames);
        memcpy(right,left,frames*sizeof *right);
        juno6_chorus_process(&p->dsp.juno.chorus,left,right,frames);ok=1;'''
new_render = '''        ok=juno6_render_stereo(&p->dsp.juno.synth,left,right,frames);
        if(ok)juno6_chorus_process(&p->dsp.juno.chorus,left,right,frames);'''
assert text.count(old_view) == 1 and text.count(old_render) == 1
runtime.write_text(text.replace(old_view,new_view).replace(old_render,new_render))
build = TREE / "tools/build.py"
text = build.read_text()
marker = 'ORIGINAL_SYNTH_BACKENDS = [\n'
assert text.count(marker) == 1
build.write_text(text.replace(marker,marker+'    "firmware/src/dsp/dnbx_core.c",\n'))

records = []
for path in [DSP / "juno6.c", DSP / "juno6.h", DSP / "dnbx_core.c", DSP / "dnbx_core.h", runtime, build]:
    rel = path.relative_to(TREE)
    old = BASE / rel
    records.append({"path":str(rel),"baseline_sha256":hashlib.sha256(old.read_bytes()).hexdigest() if old.exists() else None,
                    "fixture_sha256":hashlib.sha256(path.read_bytes()).hexdigest()})
proof = {"baseline_commit":"76040daa5b52d686a8b7b544296ba5d79610a95d",
         "candidate_provenance":json.loads((HERE / "CANDIDATE-PROVENANCE.json").read_text()),
         "files":records,"stock_chorus_sha256":hashlib.sha256(chorus.encode()).hexdigest(),
         "scope":"Real stock Juno slot replaced by candidate render/note/parameter lifecycle, with stock18 macro facade; no full original218-control or SynthOne implementation",
         "mutable_storage":"Candidate+stock18macro facade+stereo HP state inside existing backend union; chorus and other engines/FX unchanged"}
(HERE / "replacement-PROVENANCE.json").write_text(json.dumps(proof,indent=2))
print(TREE)
print(json.dumps(proof,indent=2))
