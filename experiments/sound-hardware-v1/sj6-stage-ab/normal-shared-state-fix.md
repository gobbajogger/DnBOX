# SuperJ6 Normal: fix Main amp order without reinterpreting filter state

The prior guarded native amp-order variant was active and linked, but was rejected: enabling or disabling Sub during a held note switched the active Main filter between unscaled and envelope-scaled history, making the native leaf boundary jump from stock 0.00165687 to 0.0150351. A correct Main amp order therefore cannot simply depend on an auxiliary fader being zero.

The replacement always keeps Main's amp-before-Normal state for typed Main operators and filters the auxiliary contribution separately with its existing approximate coefficient/gain law. Auxiliary history decays when input shuts off rather than being reinterpreted as Main. An initial implementation allocated new per-voice fields and overflowed app flash by 444/432 B. The linked candidate reuses the two OTA-only tail words while Normal is active. Native original mode selection already clears those words across Normal/OTA handoff, and the paired mode-switch fixture verifies the clear. Voice allocation/reuse uses the existing object initialization. No master-volume or preset change. This is still a compact filter/auxiliary diagnostic, not proof that inherited auxiliary or exact Normal coefficient behavior matches the original IPA.

## Test results

* Compiled host isolated Sub toggle: 38,776 signed output words, 16,150 exact before enable, and **14,400 identical Main IIR state words** across toggles compared with the otherwise identical Main-only arm. Auxiliary contribution is observed while active and continues a finite tail after disable.
* Actual PI32 paired leaf: live Sub on/off boundary jump **0.00165686** (stock **0.00165687**, rejected one-history patch **0.0150351**). Legacy and standalone OTA match bit-for-bit by signed output digest; corrected Main-only matches the prior amp-before variant. Six cases include a distinct active Normal→OTA→Normal handoff that verifies ownership storage clearance. Synthetic transition test, not a whole-preset or hardware listening result.
* Both actual joined links: Elite/Pro APP free **336/348 B**, audio pool unchanged **4,480 B** after reserve. Cost +288 B app versus app state-reclaim donor, no added audio pool/RAM allocation.
* Joined two-owner strict replay: Elite **30,966 µs/159 XRUNs**, Pro **23,995 µs/184 XRUNs**. Both five typed readbacks and actual measured coaudibility; no skips or bad mailboxes, immutable emulator-only NOR. This replay is not the same targeted transition test and does not establish steady whole sound or a performance gain. Still fails ≤10,809 µs and zero XRUN. Do not enable or release the feature from these receipts.

## Source and replay

Start from exact joined diagnostic with table and bounded RAMTEXT relocation, apply normal-before.patch then shared-state.patch with zero fuzz. Enable only -DDNBOX_LAB_SJ6_NORMAL_SPLIT=1 (with existing X0X_SJ6_OTA_NATIVE). Leave earlier -DDNBOX_LAB_SJ6_AMP_BEFORE_NORMAL disabled in this build. Both patches default off. The two source files named check.c below are separate tests: C include path on the native leaf uses the patched dnbx_core.c; compile to native R3 raw elf as in earlier PI32 source fixture, no contraction, ordinary 360 MHz strict replay. Dump result header and six case structs at RAM offset 0x10000.


### sj6-normal-native-fix/normal-before.patch

```diff
--- a/firmware/src/dsp/dnbx_core.c
+++ b/firmware/src/dsp/dnbx_core.c
@@ -1028,9 +1028,21 @@
                 else
 #endif
                 {
+#if DNBOX_LAB_SJ6_AMP_BEFORE_NORMAL
+                const int source_main=s->superj6_amp_range && p->oscillator_mode==DNBX_OSC_SUPERJ6
+#if X0X_SJ6_NOISE_SUB_NATIVE
+                  && s->sj6_noise_gain==0.f && s->sj6_sub_gain==0.f
+#endif
+                  && p->sub_gain==0.f;
+                if(source_main)x*=velocity*amp;
+#endif
                 lv->filter_first=fm_clampf(lv->filter_first+a*(x-lv->filter_first-p->resonance*.8f*lv->filter_second),-4,4);
                 lv->filter_second=fm_clampf(lv->filter_second+a*(lv->filter_first-lv->filter_second),-4,4);
+#if DNBOX_LAB_SJ6_AMP_BEFORE_NORMAL
+                mix[l]+=source_main?lv->filter_second:velocity*amp*lv->filter_second;
+#else
                 mix[l]+=velocity*amp*lv->filter_second;
+#endif
                 }
                 lv->phase+=lv->increment; if (lv->phase>=1) lv->phase-=1;
                 lv->subphase+=lv->increment*.5f; if (lv->subphase>=1) lv->subphase-=1;
```

### sj6-normal-shared-state/shared-state.patch

```diff
--- a/firmware/src/dsp/dnbx_core.c
+++ b/firmware/src/dsp/dnbx_core.c
@@ -998,16 +998,32 @@
                     ? factory_oscillator(lv, p->waveform, width[l], w)
                     : oscillator(lv->phase,lv->increment,p->waveform,width[l],p->oscillator_mode,w);
                 if (osc_gain[l]!=1.f) x*=osc_gain[l];
+#if DNBOX_LAB_SJ6_NORMAL_SPLIT && X0X_SJ6_OTA_NATIVE
+                const float source_osc=x;
+                float aux=(lv->subphase<.5f?1.0f:-1.0f)*p->sub_gain;
+                x+=aux;
+#else
                 x+=(lv->subphase<.5f?1.0f:-1.0f)*p->sub_gain;
+#endif
 #if X0X_SJ6_NOISE_SUB_NATIVE
                 if(!l) {
+                    
+#if DNBOX_LAB_SJ6_NORMAL_SPLIT && X0X_SJ6_OTA_NATIVE
+                    if(s->sj6_noise_gain) { const float in=s->sj6_noise_gain*sj6_noise_next(&sj6_noise);x+=in;aux+=in; }
+#else
                     if(s->sj6_noise_gain) x+=s->sj6_noise_gain*sj6_noise_next(&sj6_noise);
+#endif
                     if(s->sj6_sub_gain) {
                         float step=sj6_steps[i];
 #if X0X_SJ6_PITCH_ENV_NATIVE
                         step*=pitch_ratio;
 #endif
-                        x+=s->sj6_sub_gain*sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]);
+                        
+#if DNBOX_LAB_SJ6_NORMAL_SPLIT && X0X_SJ6_OTA_NATIVE
+                    const float in=s->sj6_sub_gain*sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]); x+=in;aux+=in;
+#else
+                    x+=s->sj6_sub_gain*sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]);
+#endif
                         v->sj6_sub_phase+=step;
                         if(v->sj6_sub_phase>=1.f) v->sj6_sub_phase-=1.f;
                     }
@@ -1028,6 +1044,26 @@
                 else
 #endif
                 {
+#if DNBOX_LAB_SJ6_NORMAL_SPLIT && X0X_SJ6_OTA_NATIVE
+                if(s->superj6_amp_range && p->oscillator_mode==DNBX_OSC_SUPERJ6){
+                    /* Never reinterpret Main history when Sub is turned on/off.
+                     * Reuse the OTA-only tail words while Normal owns this voice. The existing
+                     * mode setter clears these words at the handoff. Aux gain law is provisional;
+                     * only the distinct Main signal ownership is under test. */
+                    const float g0=velocity*amp, sx=source_osc*g0;
+                    const float resonance=p->resonance*.8f;
+                    lv->filter_first=fm_clampf(lv->filter_first+a*(sx-lv->filter_first-resonance*lv->filter_second),-4,4);
+                    lv->filter_second=fm_clampf(lv->filter_second+a*(lv->filter_first-lv->filter_second),-4,4);
+                    float y=lv->filter_second;
+                    if(aux!=0.f||lv->ota_tail[0]!=0.f||lv->ota_tail[1]!=0.f){
+                        lv->ota_tail[0]=fm_clampf(lv->ota_tail[0]+a*(aux-lv->ota_tail[0]-resonance*lv->ota_tail[1]),-4,4);
+                        lv->ota_tail[1]=fm_clampf(lv->ota_tail[1]+a*(lv->ota_tail[0]-lv->ota_tail[1]),-4,4);
+                        y+=g0*lv->ota_tail[1];
+                    }
+                    mix[l]+=y;
+                } else
+#endif
+                {
 #if DNBOX_LAB_SJ6_AMP_BEFORE_NORMAL
                 const int source_main=s->superj6_amp_range && p->oscillator_mode==DNBX_OSC_SUPERJ6
 #if X0X_SJ6_NOISE_SUB_NATIVE
@@ -1043,6 +1079,7 @@
 #else
                 mix[l]+=velocity*amp*lv->filter_second;
 #endif
+                }
                 }
                 lv->phase+=lv->increment; if (lv->phase>=1) lv->phase-=1;
                 lv->subphase+=lv->increment*.5f; if (lv->subphase>=1) lv->subphase-=1;
```

### sj6-normal-split-next/check.c

```c
#include "dnbx_core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
static dnbx_part noaux,sw;
static void start(dnbx_part *s){
 assert(dnbx_init_superj6(s,44100.f)==DNBX_OK);
 for(unsigned l=0;l<2;l++){
  dnbx_layer_params p=s->layer[l].params;
  p.oscillator_mode=DNBX_OSC_SUPERJ6;p.waveform=l?1.f/3.f:2.f/3.f;
  p.sub_gain=0;
  assert(dnbx_set_layer(s,l,&p)==DNBX_OK);
 }
 assert(dnbx_note_on(s,36,86)>=0);
}
int main(void){
 start(&noaux);start(&sw);
 const uint8_t sub[2]={0,24},none[2]={0,0};
 float a[64],b[64],x[64],y[64];
 unsigned same=0,different=0,observed=0,ntail=0,checked=0;
 for(unsigned k=0;k<600;k++){
  if(k==80){assert(dnbx_note_off(&noaux,36)==DNBX_OK);assert(dnbx_note_off(&sw,36)==DNBX_OK);}
  if(k==220){assert(dnbx_note_on(&noaux,48,109)>=0);assert(dnbx_note_on(&sw,48,109)>=0);}
  if(k==250)dnbx_set_sj6_sources(&sw,0,sub);
  if(k==280)dnbx_set_sj6_sources(&sw,0,none);
  const unsigned n=1u+(k*13u)%64u;
  assert(dnbx_render_pitch_osc_levels_velocity(&noaux,a,b,n,1.f,.808f,.808f,0.f)==DNBX_OK);
  assert(dnbx_render_pitch_osc_levels_velocity(&sw,x,y,n,1.f,.808f,.808f,0.f)==DNBX_OK);
  for(unsigned i=0;i<DNBX_VOICES;i++)for(unsigned l=0;l<2;l++){
   const dnbx_layer_voice *u=&noaux.voice[i].layer[l],*v=&sw.voice[i].layer[l];
   assert(u->filter_first==v->filter_first&&u->filter_second==v->filter_second);
   checked+=2;
  }
  for(unsigned j=0;j<n;j++){
   assert(isfinite(x[j])&&isfinite(y[j]));
   if(k<250){assert(a[j]==x[j]&&b[j]==y[j]);same+=2;}
   if(k>=250&&k<280&&a[j]!=x[j])different++;
   if(k>=280&&k<290&&a[j]!=x[j])ntail++;
   observed+=2;
  }
 }
 assert(different&&ntail&&same);
 printf("signed outputs=%u; before enable exact words=%u; Main history words identical=%u; Sub active diff=%u; post-disable tail samples=%u\n",
        observed,same,checked,different,ntail);
}
```

### sj6-normal-shared-state/leaf/check.c

```c
#include <stdint.h>
#include "dnbx_core.c"
#include "fm1_time.h"
static dnbx_part s;
static float l[64],r[64];
static uint32_t hash(uint32_t x,float f){
  union {float f;uint32_t u;} p={f};
  return (x^p.u)*0x01000193u;
}
static unsigned synth(unsigned mode,volatile uint32_t *report){
 unsigned bad=0;
 if((mode==1?dnbx_init(&s,44100.f):dnbx_init_superj6(&s,44100.f))!=DNBX_OK)return 1;
 for(unsigned i=0;i<2;++i){
  dnbx_layer_params p=s.layer[i].params;
  p.oscillator_mode=mode==1?DNBX_OSC_STOCK_BLEP:DNBX_OSC_SUPERJ6;
  p.waveform=i?1.f/3.f:2.f/3.f;p.amp.release_s=.45f;
  if(dnbx_set_layer(&s,i,&p)!=DNBX_OK)return 1;
 }
 const uint8_t sub[2]={0,24},none[2]={0,0};
 if(mode==2||mode==5)dnbx_set_sj6_sources(&s,0,sub);
 if(mode==3)s.sj6_ota=1;
 if(dnbx_note_on(&s,36,86)<0)return 1;
 uint32_t h=0x811c9dc5u,active=0;float mx=0.f,djump=0.f,last=0.f;
 for(unsigned k=0;k<460;k++){
  if(k==80)bad+=dnbx_note_off(&s,36)!=DNBX_OK;
  if(k==220)bad+=dnbx_note_on(&s,48,109)<0;
  if(mode==4&&k==250)dnbx_set_sj6_sources(&s,0,sub);
  if(mode==5&&(k==250||k==280)){
      if(!dnbx_set_sj6_ota(&s,k==250?1u:0u))return 1;
      for(unsigned i=0;i<DNBX_VOICES;i++)for(unsigned j=0;j<DNBX_LAYERS;j++){
        const dnbx_layer_voice *v=&s.voice[i].layer[j];
        bad+=v->ota_tail[0]!=0.f||v->ota_tail[1]!=0.f;
      }
  }
  if(mode==4&&k==280)dnbx_set_sj6_sources(&s,0,none);
  const unsigned n=1u+(k*13u)%64u;
  if(dnbx_render_pitch_osc_levels_velocity(&s,l,r,n,1.f,.808f,.808f,0.f)!=DNBX_OK)return 1;
  if(mode==4&&(k==250||k==280))djump+=fm_fabsf(l[0]-last);
  for(unsigned j=0;j<n;j++){
   h=hash(hash(h,l[j]),r[j]); active+=(l[j]!=0.f||r[j]!=0.f);last=l[j];
   float a=fm_fabsf(l[j]);if(a>mx)mx=a;
  }
 }
 report[0]=h;report[1]=active;
 union{float f;uint32_t u;} v;
 v.f=mx;report[2]=v.u;v.f=djump;report[3]=v.u;
 return bad||!active;
}
void bench_start(void){
 volatile uint32_t *r=(void*)0x01c10000;unsigned fail=0;
 r[0]=0;fm1_time_init();uint32_t start=fm1_ticks();
 for(unsigned i=0;i<6;i++)fail+=synth(i,r+4+4*i);
 r[1]=fail;r[2]=fm1_ticks()-start;r[3]=DNBOX_LAB_SJ6_NORMAL_SPLIT;r[0]=0x4e4f524d;
}
void *memset(void *p,int v,size_t n){volatile unsigned char *d=p;while(n--)*d++=(unsigned char)v;return p;}
void *memcpy(void *p,const void *s,size_t n){volatile unsigned char *d=p;const unsigned char *a=s;while(n--)*d++=*a++;return p;}
```

### sj6-normal-shared-state/host-result.txt

```text
signed outputs=38776; before enable exact words=16150; Main history words identical=14400; Sub active diff=912; post-disable tail samples=12
```

### sj6-normal-shared-state/leaf/result.json

```json
{
  "arms": [
    "Main-only",
    "legacy",
    "Sub active",
    "OTA",
    "live Sub toggles",
    "live Normal\u2192OTA\u2192Normal"
  ],
  "prior_narrow_jump": 0.015035061165690422,
  "results": [
    {
      "ticks": 20015847,
      "cases": [
        {
          "signed_hash": 184968559,
          "audible": 14854,
          "peak": 0.04882042855024338,
          "sum_boundary_jump": 0.0
        },
        {
          "signed_hash": 734882783,
          "audible": 14854,
          "peak": 0.09899353981018066,
          "sum_boundary_jump": 0.0
        },
        {
          "signed_hash": 3940542287,
          "audible": 14854,
          "peak": 0.04990621656179428,
          "sum_boundary_jump": 0.0
        },
        {
          "signed_hash": 227155967,
          "audible": 14854,
          "peak": 0.03612356632947922,
          "sum_boundary_jump": 0.0
        },
        {
          "signed_hash": 2076996863,
          "audible": 14854,
          "peak": 0.04882042855024338,
          "sum_boundary_jump": 0.0016568666324019432
        },
        {
          "signed_hash": 2266668567,
          "audible": 14854,
          "peak": 0.04990621656179428,
          "sum_boundary_jump": 0.0
        }
      ]
    },
    {
      "ticks": 20642005,
      "cases": [
        {
          "signed_hash": 3883661755,
          "audible": 14854,
          "peak": 0.04879649728536606,
          "sum_boundary_jump": 0.0
        },
        {
          "signed_hash": 734882783,
          "audible": 14854,
          "peak": 0.09899353981018066,
          "sum_boundary_jump": 0.0
        },
        {
          "signed_hash": 3454648215,
          "audible": 14854,
          "peak": 0.04988228529691696,
          "sum_boundary_jump": 0.0
        },
        {
          "signed_hash": 227155967,
          "audible": 14854,
          "peak": 0.03612356632947922,
          "sum_boundary_jump": 0.0
        },
        {
          "signed_hash": 3166132671,
          "audible": 14854,
          "peak": 0.04879649728536606,
          "sum_boundary_jump": 0.001656860113143921
        },
        {
          "signed_hash": 3233430805,
          "audible": 14854,
          "peak": 0.04988228529691696,
          "sum_boundary_jump": 0.0
        }
      ]
    }
  ]
}
```

### sj6-normal-shared-state/replay-elite.json

```json
{"worst_us": 30966, "halves": 410, "raw_late": 159, "xruns": 159, "skipped_cycles": 0, "peak": 13138, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1755, "finished": 1754, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 450, "b": 450}, "joint_audible_spans": 450, "owners": {"count": 5, "owners": [{"ms": 2694, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3240, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4126, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5792, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7857, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```

### sj6-normal-shared-state/replay-pro.json

```json
{"worst_us": 23995, "halves": 419, "raw_late": 184, "xruns": 184, "skipped_cycles": 0, "peak": 13143, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1797, "finished": 1796, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 472, "b": 472}, "joint_audible_spans": 472, "owners": {"count": 5, "owners": [{"ms": 2672, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3253, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4120, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5696, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7837, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```

### sj6-normal-shared-state/run-native-check.sh

```bash
#!/usr/bin/env bash
set -euo pipefail
R=$(cd "$(dirname "$0")/.." && pwd)
S="$R/s1-organ-wet-recovery/native-ed-brk-recompose"
H="$R/restored-harness-inspect/helpers"
O="$R/remote-typed-load"
for d in "$R/sj6-normal-shared-state"; do
  e="$d/tree/joint-diagnostic-later/elite/never-install.elf"
  p="$d/tree/joint-diagnostic-later/pro/never-install.elf"
  (
    exec 9>/tmp/dnbox-gate-firmware.lock
    flock -x 9
    python "$H/guarded_replay.py" --workspace "$R" --archive "$R/restored-harness-inspect/dot-gate-harness.zip" --elite "$e" --elite-sha "$(sha256sum "$e"|cut -d' ' -f1)" --pro "$p" --pro-sha "$(sha256sum "$p"|cut -d' ' -f1)" --run --jobs 1 --offsets=-1 > "$d/s1-ladder-cold.log"
  )
  W=$(head -n1 "$d/s1-ladder-cold.log"); [[ $W == /tmp/dnbox-emulator-only-* ]]
  for m in elite pro; do
    PYTHONPATH="$H" python "$S/seed.py" "$W/$m-virtual-nor.bin" "$d/tree/joint-diagnostic-later/$m/never-install.elf" "$W/$m-offset--1/ram.bin" "$O/tree/build/smk37-elite/x0x.elf" "$O/loaded-smoke-elite/ram.bin" "$S/fixture" "$W/$m-two-owners.bin"
  done
  cp "$d/s1-ladder-cold.log" "$d/cold.log"
  bash "$d/run-actual.sh"
done
```
