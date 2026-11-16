from pathlib import Path
import hashlib,json,re
r=Path(__file__).resolve().parent;master=Path('synthone-compact-native-20261115/native-inputs/master/AudioKitSynthOne/DSP/Kernel');up=r/'upstream';shim=r/'compat/AudioKit';shim.mkdir(parents=True,exist_ok=True)
(shim/'AKInterop.h').write_text('#pragma once\n#define AK_ENUM(N) enum N\n');(shim/'AKSoundpipeKernel.hpp').write_text('#pragma once\n#include <array>\n#include <memory>\nextern "C" {\n#include "soundpipe.h"\n}\n')
p=(master/'S1DSPKernel+process.mm').read_text();body=p[p.index('        // MONO: NoteState render output'):p.rfind('    }')];assert body.endswith('outR[frameIndex] = widenOutR;\n');body=body.replace('float synthOut = outL[frameIndex];','float synthOut = input;')
init=(master/'S1DSPKernel.mm').read_text();stereo=init[init.index('    //STEREO'):init.index('    noteStates =')];restore=init[init.index('    *phaser0->MinNotch1Freq'):init.index('    // Reserve arp note cache')]
rows=json.load(open('engine-parameter-ux-contract-20261115/SYNTHONE-150-METADATA.private.json'))['rows'];assert len(rows)==150
arrs=[]
for key in ['minimum','default','maximum']:
 arrs.append('static constexpr uint32_t '+key+'_words[150]={'+','.join('0x'+x[key+'_f32_hex']+'u'for x in rows)+'};')
reset=(master/'S1DSPKernel+reset.mm').read_text();reset_calls=reset[reset.index('    sp_vdelay_reset'):reset.index('\n}',reset.index('    sp_vdelay_reset'))]
header='''// Original-source full FX reference, isolated host experiment, NOT target admission.
#pragma once
#include <array>
#include <memory>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include "fx_compressor.hpp"
#define nil nullptr
struct FxSpOwner { sp_data *sp=nullptr; FxSpOwner(int rate){sp_create(&sp);sp->sr=rate;sp->nchan=2;}~FxSpOwner(){sp_destroy(&sp);} };
struct SourceFx:FxSpOwner {
 DSPParameters parameters{};float lows[150]{},highs[150]{};
 sp_ftbl*sine=nullptr;sp_osc*panOscillator=nullptr;sp_pan2*pan=nullptr;sp_phaser*phaser0=nullptr;
 sp_moogladder*loPassInputDelayL=nullptr,*loPassInputDelayR=nullptr;
 sp_vdelay*delayL=nullptr,*delayR=nullptr,*delayRR=nullptr,*delayFillIn=nullptr;
 sp_crossfade*delayCrossfadeL=nullptr,*delayCrossfadeR=nullptr,*revCrossfadeL=nullptr,*revCrossfadeR=nullptr;
 sp_revsc*reverbCostello=nullptr;sp_buthp*butterworthHipassL=nullptr,*butterworthHipassR=nullptr;sp_delay*widenDelay=nullptr;
 S1Compressor<compressorMasterRatio,compressorMasterThreshold,compressorMasterAttack,compressorMasterRelease,compressorMasterMakeupGain>mCompMaster;
 S1Compressor<compressorReverbInputRatio,compressorReverbInputThreshold,compressorReverbInputAttack,compressorReverbInputRelease,compressorReverbInputMakeupGain>mCompReverbIn;
 S1Compressor<compressorReverbWetRatio,compressorReverbWetThreshold,compressorReverbWetAttack,compressorReverbWetRelease,compressorReverbWetMakeupGain>mCompReverbWet;
 float bitcrushIncr=1.f,bitcrushIndex=0.f,bitcrushSampleIndex=0.f,bitcrushValue=0.f;
 SourceFx(const SourceFx&)=delete;SourceFx&operator=(const SourceFx&)=delete;
 explicit SourceFx(int rate):FxSpOwner(rate),mCompMaster(sp,&parameters),mCompReverbIn(sp,&parameters),mCompReverbWet(sp,&parameters){
'''+ '\n'.join(arrs)+'''
 for(unsigned i=0;i<150;++i){std::memcpy(&lows[i],minimum_words+i,4);std::memcpy(&highs[i],maximum_words+i,4);std::memcpy(&parameters[i],default_words+i,4);}
 sp_ftbl_create(sp,&sine,4096);sp_gen_sine(sp,sine);sp_phaser_create(&phaser0);sp_phaser_init(sp,phaser0);sp_osc_create(&panOscillator);sp_osc_init(sp,panOscillator,sine,0.f);sp_pan2_create(&pan);sp_pan2_init(sp,pan);
'''+stereo+restore+'''
 }
 ~SourceFx(){
 sp_osc_destroy(&panOscillator);sp_pan2_destroy(&pan);sp_phaser_destroy(&phaser0);sp_moogladder_destroy(&loPassInputDelayL);sp_moogladder_destroy(&loPassInputDelayR);sp_vdelay_destroy(&delayL);sp_vdelay_destroy(&delayR);sp_vdelay_destroy(&delayRR);sp_vdelay_destroy(&delayFillIn);sp_crossfade_destroy(&delayCrossfadeL);sp_crossfade_destroy(&delayCrossfadeR);sp_crossfade_destroy(&revCrossfadeL);sp_crossfade_destroy(&revCrossfadeR);sp_revsc_destroy(&reverbCostello);sp_buthp_destroy(&butterworthHipassL);sp_buthp_destroy(&butterworthHipassR);sp_delay_destroy(&widenDelay);sp_ftbl_destroy(&sine);
 }
 void source_reset_delays(){\n'''+reset_calls+'''\n }
 void source_restore_effect_controls(){\n'''+restore+'''\n }
 int sampleRate()const{return sp->sr;}float maximum(S1Parameter i)const{return highs[i];}float getSynthParameter(S1Parameter i)const{return parameters[i];}float clampedValue(S1Parameter i,float v)const{return std::min(std::max(v,lows[i]),highs[i]);}
 void frame(float input,float*outL,float*outR,float lfo1_0_1,float lfo1_1_0,float lfo2_0_1,float lfo2_1_0){const unsigned frameIndex=0;float lfo3_0_1=.5f*(lfo1_0_1+lfo2_0_1),lfo3_1_0=.5f*(lfo1_1_0+lfo2_1_0);
'''+body+''' }
};
#undef nil
'''
(r/'source_fx.hpp').write_text(header)
d=dict(scope='Original FX source extraction, original arithmetic/call order, DSP cold allocation preserved. Isolated host reference, not caller-owned/native-fit admission.',source='6466a3715c96b7ecf1dd255a218cbd571408a314',process_original_sha256=hashlib.sha256(p.encode()).hexdigest(),body_start_marker='// MONO: NoteState render output',body_change='Replace sole synthOut source outL[frameIndex] with supplied mono input; frameIndex fixed0, explicit original LFO inputs supplied. Pre-FX notes/ports/LFO/sequence remain outside this component.',source_fx_sha256=hashlib.sha256(header.encode()).hexdigest(),defaults='All150 positional binary32 rows, including136=1.88, authenticated previous source query',limits=['No full original app runtime','No whole-source Scheduler/FX composition or matched port evidence yet','No numerical policy/target link/storage/CPU/device acceptance','Original4x10sec delay buffers retained, not reduced or moved'])
(r/'FULL-FX-DERIVATION.json').write_text(json.dumps(d,indent=2)+'\n');print('Generated original fullFX reference',len(header.encode()),d['source_fx_sha256'])
