#pragma once
#include <cstdint>
namespace original_conductor_v1 {
enum class EntropyStatus {Unsupported,Complete,MissingProvider,SourceFailure,BudgetExceeded};
struct EntropyProvider {void* context;bool(*next_word)(void*,std::uint64_t&) noexcept;};
struct EntropyResult {EntropyStatus status;std::uint64_t requests;};
// Cold serialized caller owns a valid output array disjoint from provider context
// and all provider-mutated storage for the entire call. No pointer admission claim.
// Each provider call represents one eight-byte random request, not a PRNG guess.
// Output is committed only on completion; provider consumption is NOT rolled back.
// A finite request budget is an explicit fail-closed adapter limit, not an original
// bound. The actual source rejection loops have no finite worst-case request count.
EntropyResult build_entropy_table12(EntropyProvider,std::uint64_t request_budget,float (&output)[256]) noexcept;
EntropyResult draw_held_sample(EntropyProvider,std::uint64_t request_budget,double& output) noexcept;
}
