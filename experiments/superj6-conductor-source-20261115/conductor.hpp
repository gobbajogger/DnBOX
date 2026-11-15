#pragma once
#include <cstddef>
#include <cstdint>

namespace original_conductor_v1 {
enum class Status { Unsupported, MainPassOnly, State, Clock, PendingAck, Invalid, Capacity, MissingCapability, MissingPow, MissingExp, MathFailure, AwaitingAck, StaleAck };
enum Capability : unsigned { ControlApply=1, DelayPrepare=2, DeferredVolume=4, CutoffApply=8, GlobalDispatch=16 };
enum class Op { ApplyControl, DelayEnablePrepare, DeferVolume, CutoffApply, GlobalDispatch, TickLfo };
enum class Domain { NormalizedControl, ElapsedSeconds };
enum class Receiver { InstrumentControl, CoreCutoff, ConductorGlobal, Conductor };
struct Identifier {
 const char* pointer;std::size_t length;
 Identifier() noexcept:pointer(nullptr),length(0){}
 Identifier(const char* p,std::size_t n) noexcept:pointer(p),length(n){}
 template<std::size_t N> Identifier(const char (&text)[N]) noexcept:pointer(text),length(text[N-1]=='\0'?N-1:N){}
 const char* data()const noexcept{return pointer;}
 std::size_t size()const noexcept{return length;}
 bool operator==(const char*)const noexcept;
};
struct Record { Identifier identifier; double value; };
struct Command { Op op; Domain domain; Receiver receiver; std::uint16_t id; double value; std::uint64_t generation; std::size_t order; double delay; std::uint64_t token; };
// C++11 cannot encode exception specifications in alias declarations.
// These providers MUST NOT throw, reenter, or mutate controller state. The
// caller enforces this contract; calls remain inside noexcept implementations.
// Native builds retain -fno-exceptions. Removing illegal alias syntax does not
// grant permission for throwing callbacks or alter any numerical expression.
using Pow = double(*)(double,double);
using Exp = double(*)(double);
struct Providers { unsigned capabilities; Pow pow; Exp exp; };
// Cold serialized API. Caller owns valid, readable immutable records/string spans;
// writable command/count spans are disjoint and remain alive throughout the call.
// This plans only the authenticated MAIN control pass, never the full loader/DSP.
Status plan_main(const Record*,std::size_t,std::uint64_t,bool,Providers,Command*,std::size_t,std::size_t&) noexcept;
Status query_default(std::uint16_t,Providers,float&) noexcept;
struct Cutoff { std::uint64_t generation; float current,target; bool active; std::uint64_t token; bool awaiting;
 Cutoff(std::uint64_t g=0,float c=0,float t=0,bool a=false,std::uint64_t k=0,bool w=false) noexcept:generation(g),current(c),target(t),active(a),token(k),awaiting(w){}
};
Status cutoff_step(const Cutoff&,double,Providers,Command*,std::size_t,std::size_t&,Cutoff&) noexcept;
Status cutoff_ack(const Cutoff&,std::uint64_t,std::uint64_t,float,Cutoff&) noexcept;
Status free_block_time(std::uint32_t,double,double&) noexcept;
struct Beat { std::uint64_t generation; double last,accumulator;
 Beat(std::uint64_t g=0,double l=0,double a=0) noexcept:generation(g),last(l),accumulator(a){}
};
Status beat_catchup(const Beat&,double,double,bool,Command*,std::size_t,std::size_t&,Beat&) noexcept;
struct Note { std::uint8_t modes[3]; bool retriggers[3]; bool armed,retrigger4,new_note; bool delay_finished[3]; double delay_passed[3]; };
Status enter_note(const Note&,std::size_t,Note&) noexcept;
}
namespace original_conductor_v1 {
struct RestoreRoute { std::uint16_t destination[2];float base[2];bool enabled,retrigger,snapback; };
// collection entries are caller-authenticated parameter identity -> global ID;
// duplicate identities are searched in order. 218 is the no-global sentinel.
struct RestoreControl { std::uint16_t parameter,global; };
struct RestoreState {std::uint64_t generation;RestoreRoute routes[3];bool cycles[3];};
struct RestoreCommand {enum class Kind {CutoffTarget,ParameterValue,GlobalDispatch};Kind kind;std::size_t collection_index;std::uint16_t id;float value;std::uint64_t generation;std::size_t order;};
enum RestoreCapability : unsigned { SetCutoffTarget=1,SetParameterFlagZero=2,DispatchGlobal=4 };
Status restore(const RestoreState&,bool forced_third,const RestoreControl*,std::size_t,unsigned,RestoreCommand*,std::size_t,std::size_t&,RestoreState&) noexcept;
}
namespace original_conductor_v1 {
enum class DeferredStatus {Unsupported,ExpiredOwner,MissingCapability,Invalid,LockFailed,StaleGeneration,ApplyFailed,Applied};
struct DeferredBackend {
 void* context;
 bool(*lock)(void*) noexcept;
 std::uint64_t(*generation)(void*) noexcept;
 bool(*apply_volume)(void*,float) noexcept;
 void(*clear_pending)(void*) noexcept;
 void(*unlock)(void*) noexcept;
};
// Caller pins the weak owner's lifetime for this cold serialized call. No task is
// scheduled here: the executor must invoke it after the command's 0.15s delay.
// All callbacks are nonthrowing. clear_pending occurs only after successful apply.
DeferredStatus execute_deferred_volume(bool owner_alive,std::uint64_t captured_generation,float captured_value,DeferredBackend) noexcept;
}
