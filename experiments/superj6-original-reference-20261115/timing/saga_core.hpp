// Isolated original-Saga behavioral port. Own source; see README.md.
// Static-derived provenance and unqualified integration boundaries are documented.
#pragma once
#include "f64_bits.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>
namespace j6_saga {
constexpr std::array<float,18> divisions{{.0625f,.09375f,.125f,.1875f,.25f,.375f,.5f,.75f,1,1.5f,2,3,4,8,12,16,24,32}};
constexpr std::array<unsigned,10> pad_x{{144,26,7,88,96,100,15,72,104,61}};
constexpr std::array<unsigned,10> pad_y{{66,27,8,89,97,101,13,73,103,60}};
enum class Status { ok, invalid_input, unresolved_random };
inline std::optional<float> duration(float stored) {
 if(!std::isfinite(stored))return {};
 const float index=std::round((1.f-stored)*17.f);
 if(!(index>=0.f&&index<18.f))return {};
 return divisions[static_cast<unsigned>(index)];
}
inline std::optional<float> rate_knob(float k) {
 if(!std::isfinite(k)||k<0.f||k>1.f)return {};
 const float shifted=float(k)*.7f+.3f;
 const float stored=std::round(shifted/(1.f/17.f))*(1.f/17.f);
 if(!duration(stored))return {};
 return stored;
}
// Beat positions are IEEE binary64 bit patterns. The 1e-9 grid bias is applied
// with integer arithmetic so offsets such as 6e-10 match the binary64 snap.
inline std::optional<std::uint64_t> snap_f64(std::uint64_t beat,std::uint64_t d,std::uint64_t offset) {
 using namespace original_conductor_v1;
 constexpr std::uint64_t kEps=0x3e112e0be826d695ULL;
 if(!f64_finite(beat)||!f64_finite(d)||!f64_finite(offset)||!f64_gt(d,0))return {};
 const std::uint64_t q=f64_sub(f64_div(f64_sub(beat,offset),d),kEps);
 const std::uint64_t value=f64_add(offset,f64_mul(d,f64_ceil(q)));
 if(!f64_finite(q)||!f64_finite(value))return {};
 return value;
}
inline std::optional<float> snap(float beat,float d,float offset) {
 using namespace original_conductor_v1;
 auto exact=snap_f64(f64_from_f32(beat),f64_from_f32(d),f64_from_f32(offset));
 if(!exact)return {};
 return f64_to_f32(*exact);
}
struct Pool { Status status{Status::ok};std::vector<int> notes; };
inline Pool arp_pool(const std::vector<std::uint8_t>& held,float octave,float interval,float raw) {
 if(!std::isfinite(octave)||!std::isfinite(interval)||!std::isfinite(raw)||octave<0.f||octave>3.f||interval<0.f||interval>12.f||raw<0.f||raw>=7.f)return {Status::invalid_input,{}};
 const int mode=int(raw),tiers=int(octave),semitones=int(interval);
 if(mode>=4)return {Status::unresolved_random,{}};
 Pool out;
 if(held.size()>std::numeric_limits<std::size_t>::max()/std::size_t(tiers+1)/4)return {Status::invalid_input,{}};
 for(auto n:held)for(int o=0;o<=tiers;++o)out.notes.push_back(int(n)+o*semitones);
 std::sort(out.notes.begin(),out.notes.end());
 if(mode==1)std::reverse(out.notes.begin(),out.notes.end());
 else if(mode==2||mode==3) {
  const auto n=out.notes.size();
  if(n>2)for(std::size_t i=n-2;i>0;--i)out.notes.push_back(out.notes[i]);
  if(mode==3) { std::vector<int> doubled;doubled.reserve(out.notes.size()*2);for(int n:out.notes){doubled.push_back(n);doubled.push_back(n);}out.notes=std::move(doubled); }
 }
 return out;
}
inline std::uint8_t clip_note(int n) { return std::uint8_t(std::max(0,std::min(127,n))); }
struct Scale { bool enabled{false};std::uint16_t absolute_pitch_classes{0xfff}; };
inline std::optional<std::uint8_t> transform_note(int n,Scale scale={}) {
 n=clip_note(n);
 if(!scale.enabled)return std::uint8_t(n);
 const unsigned mask=scale.absolute_pitch_classes&0xfff;
 if(!mask)return {};
 auto allowed=[mask](int p){return (mask>>(p%12))&1u;};
 if(allowed(n))return std::uint8_t(n);
 for(int d=1;d<=127;++d) {
  if(n-d>=0&&allowed(n-d))return std::uint8_t(n-d);
  if(n+d<=127&&allowed(n+d))return std::uint8_t(n+d);
 }
 return {};
}
inline std::optional<std::vector<std::uint8_t>> sequence_notes(const std::vector<std::uint8_t>& held,float offset,float toggle,Scale scale={}) {
 if(!std::isfinite(offset)||!std::isfinite(toggle)||offset< -24.f||offset>24.f||(scale.enabled&&!(scale.absolute_pitch_classes&0xfff)))return {};
 std::vector<std::uint8_t> result;
 if(toggle>=1.f)for(auto n:held){auto out=transform_note(int(n)+int(offset),scale);if(!out)return {};result.push_back(*out);}
 return result;
}
inline std::optional<std::array<unsigned,2>> pad_targets(int raw) {
 if(raw<0||raw>=10)return {};
 return std::array<unsigned,2>{{pad_x[raw],pad_y[raw]}};
}
inline std::optional<int> next_pad(int current,int other,int direction) {
 if(current<0||current>=10||other<0||other>=10||(direction!=1&&direction!=-1))return {};
 int n=(current+direction+10)%10;
 if(n==other)n=(n+direction+10)%10;
 return n;
}
inline std::optional<unsigned> destination_mask(float raw) {
 if(!std::isfinite(raw)||raw<0.f||raw>=3.f)return {};
 constexpr unsigned masks[]={15,13,10};return masks[int(raw)];
}
struct Settings {
 bool arp_on{true},seq_on{false};
 float arp_type{0},extra_octaves{1},interval{12},tempo{120},running{0};
 float steps{16},gate{.99f},division{.5f},destination{0};
 std::uint8_t velocity{87};
 std::array<float,16> enabled{},offset{};
 Settings(){enabled.fill(1.f);offset.fill(0.f);}
};
struct State {
 Settings settings;
 int step{0};
 float next_step{0},next_gate_end{0},previous_step{0},previous_beat{0};
 std::vector<std::uint8_t> held,generated;
 bool reset_if_no_held{true};
 std::size_t arp_index{0};int arp_direction{1};
 bool transport_change{true};std::uint8_t old_note{0},new_note{60};
};
struct Event { std::uint8_t note,velocity,channel{0}; };
struct Frame { std::uint32_t frames;float sample_rate,beat;bool synced;float grid_offset{0};Scale scale{}; };
struct Result { Status status;State next;std::vector<Event> events;bool admitted_step{false}; };
inline Result process(const State& input,Frame f) {
 auto fail=[&](Status error){return Result{error,input,{},false};};
 const auto& c=input.settings;auto d=duration(c.division);
 if(!d||!std::isfinite(f.sample_rate)||f.sample_rate<=0.f||!std::isfinite(f.beat)||!std::isfinite(f.grid_offset)||!std::isfinite(c.tempo)||c.tempo<=0.f||!std::isfinite(c.running)||!std::isfinite(c.steps)||c.steps<1.f||c.steps>16.f||!std::isfinite(c.gate)||c.gate<0.f||c.gate>1.f||input.step<0||input.step>=16||!std::isfinite(input.next_step)||!std::isfinite(input.next_gate_end)||!std::isfinite(input.previous_step)||!std::isfinite(input.previous_beat)||(f.scale.enabled&&!(f.scale.absolute_pitch_classes&0xfff)))return fail(Status::invalid_input);
 float comparison=f.beat;
 if(!f.synced) {
  const float seconds=float(f.frames)/f.sample_rate;
  const float beats_per_second=c.tempo/60.f;
  comparison+=seconds*beats_per_second;
 }
 if(!std::isfinite(comparison))return fail(Status::invalid_input);
 Result result{Status::ok,input,{},false};auto& q=result.next;
 auto flush=[&](){for(auto n:q.generated)result.events.push_back({n,0,0});q.generated.clear();};
 if(q.next_gate_end<=comparison)flush();
 auto reset=[&](bool quantized)->bool {
  auto next=quantized?snap(f.beat,*d,f.grid_offset):std::optional<float>{f.beat};
  if(!next)return false;
  q.step=0;q.arp_index=0;q.next_step=*next;q.previous_step=f.beat;q.previous_beat=f.beat;q.next_gate_end=f.beat;flush();return true;
 };
 if(f.synced&&(q.transport_change||f.beat<q.previous_beat)) {
  if(!reset(true))return fail(Status::invalid_input);
  q.transport_change=false;
 } else if(q.next_step<=f.beat&&q.previous_beat<=q.next_step&&q.transport_change) {
  if(!reset(false))return fail(Status::invalid_input);
  q.transport_change=false;
 } else if(f.beat<q.previous_beat) { if(!reset(false))return fail(Status::invalid_input); }
 if(c.running<=0.f)return result;
 if(q.next_step>comparison){q.previous_beat=f.beat;return result;}
 if(c.arp_on) {
  if(!q.held.empty()) {
   auto pool=arp_pool(q.held,c.extra_octaves,c.interval,c.arp_type);
   if(pool.status!=Status::ok)return fail(pool.status);
   if(pool.notes.empty())return fail(Status::invalid_input);
   auto n=transform_note(pool.notes[q.arp_index%pool.notes.size()],f.scale);if(!n)return fail(Status::invalid_input);
   q.new_note=*n;q.generated.push_back(*n);result.events.push_back({*n,c.velocity,0});
   // Original selection uses modulo, but postselection increments then zeros.
   if(q.arp_index>=std::size_t(std::numeric_limits<std::int64_t>::max()))return fail(Status::invalid_input);
   ++q.arp_index;
   if(q.arp_index>=pool.notes.size())q.arp_index=0;
  }
 } else {
  auto notes=sequence_notes(q.held,c.offset[q.step],c.enabled[q.step],f.scale);if(!notes)return fail(Status::invalid_input);
  for(auto n:*notes){q.new_note=n;q.generated.push_back(n);result.events.push_back({n,c.velocity,0});}
 }
 const float old=q.next_step;
 q.next_step=old+*d;q.next_gate_end=old+*d*c.gate;
 if(!std::isfinite(q.next_step)||!std::isfinite(q.next_gate_end))return fail(Status::invalid_input);
 q.previous_step=old;q.old_note=q.new_note;
 ++q.step;if(q.step>int(c.steps-1.f))q.step=0;
 q.previous_beat=f.beat;result.admitted_step=true;return result;
}
}
