// Original selected-component noise reference; not a guarded/realtime API.
#pragma once
#include <cstdint>
#include <cmath>
namespace DunneCore {
struct OriginalWhiteNoise {
 std::uint32_t seed=0;
 static std::uint32_t mix(std::uint32_t x) {x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;return x^(x>>16);}
 static std::uint32_t fromCounter(std::uint32_t x){return x?mix(x):0x6d2b79f5u;}
 void reset(){const std::uint32_t x=seed+0x9e3779b9u;seed=x?mix(x):0x6d2b79f5u;}
 // Default original coloration only: stages0, output gain1, rate1.
 // Custom coloration/filtering and rate interpolation remain outside this API.
 float sample(){std::uint32_t x=seed;x^=x<<13;x^=x>>17;x^=x<<5;seed=x;return std::fma(float(x>>8),1.1920930376163597e-7f,-1.f);}
};
}
