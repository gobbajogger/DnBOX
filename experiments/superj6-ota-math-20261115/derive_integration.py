from pathlib import Path
import hashlib,json,re
R=Path(__file__).resolve().parent;H=lambda b:hashlib.sha256(b).hexdigest()
original=(R/'baseline/OriginalOTAFilter.h').read_bytes();assert H(original)=='69eef11c13960ceaedc2d11903c762f906a7a5432c08c4484683765c6f71d833'
for name,want in[('fma32.c','71a67e9a07caa496f2b55c4f1630fcde707dfe3d03730693b9e28dc6aa0da8e2'),('fma32.h','be30ef7f2e2afd6436cd17676d754b5c495ffce7782f927dff933ce246ba635c')]:assert H((R/'math'/name).read_bytes())==want
src=original.decode();assert src.count('std::fma(')==36 and src.count('float(cutoff)')==1 and src.count('float(normalizedRes)')==1
stage=src.replace('#include <cmath>','#include <cmath>\n#include "ota_math.hpp"').replace('std::fma(','ota_f32_math::fma(');(R/'baseline/OriginalOTA-FMA-only.h').write_text(stage)
new=stage.replace('float(cutoff)','ota_f32_math::ingress(cutoff)').replace('float(normalizedRes)','ota_f32_math::ingress(normalizedRes)');(R/'candidate/OriginalOTAFilter.h').write_text(new)
back=new.replace('#include <cmath>\n#include "ota_math.hpp"','#include <cmath>').replace('ota_f32_math::fma(','std::fma(').replace('ota_f32_math::ingress(cutoff)','float(cutoff)').replace('ota_f32_math::ingress(normalizedRes)','float(normalizedRes)');assert back.encode()==original
p=R/'upstream-new/f64_to_f32.c';b=p.read_bytes();assert len(b)==3477 and hashlib.sha1(b'blob '+str(len(b)).encode()+b'\0'+b).hexdigest()=='6074bb8455f0500c3275cf9c2466d2cdfe3d55d0'
s=b.decode();s=re.sub(r'^#include[^\n]*\n','',s,flags=re.M);s=s.replace('float32_t f64_to_f32( float64_t a )','static float32_t ota_convert64( struct fma32_context *ctx, float64_t a )');s=s.replace('softfloat_shortShiftRightJam64(','fma32_shortShiftRightJam64(').replace('softfloat_roundPackToF32(','fma32_roundPackToF32( ctx, ')
prefix='''/* Integer ingress representation adapter; generated from exact public SoftFloat.
 * Accepted FMA32 source is included unchanged to share context-local roundPack. */
#include "numeric_boundary.h"
#include "fma32.c"
_Static_assert(sizeof(double)==8 && DBL_MANT_DIG==53 && DBL_MAX_EXP==1024 && DBL_MIN_EXP==-1021,"IEEE binary64 ABI required");
typedef struct {uint64_t v;} float64_t;
union ui64_f64 {uint64_t ui;float64_t f;};
struct commonNaN {char _unused;};
#define signF64UI(a) ((bool)((uint64_t)(a)>>63))
#define expF64UI(a) ((int_fast16_t)((a)>>52)&0x7FF)
#define fracF64UI(a) ((a)&UINT64_C(0x000FFFFFFFFFFFFF))
#define softfloat_f64UIToCommonNaN(uiA,zPtr) if(!((uiA)&UINT64_C(0x0008000000000000))) fma32_raiseFlags(ctx,softfloat_flag_invalid)
#define softfloat_commonNaNToF32UI(aPtr) ((uint_fast32_t)defaultNaNF32UI)
'''
foot='''
fma32_result f64_to_f32_evaluate(uint64_t bits){
 struct fma32_context ctx={0};float64_t a={bits};float32_t z=ota_convert64(&ctx,a);fma32_result r={z.v,ctx.flags};return r;
}
float f64_to_f32_rne(double value){union{double f;uint64_t u;}a;union{uint32_t u;float f;}z;a.f=value;z.u=f64_to_f32_evaluate(a.u).bits;return z.f;}
''';(R/'math/numeric_boundary.c').write_text(prefix+s+foot)
j=dict(original_OTA_sha256=H(original),accepted_FMA_source_sha256=H((R/'math/fma32.c').read_bytes()),conversion_upstream_git_blob='6074bb8455f0500c3275cf9c2466d2cdfe3d55d0',OTA_FMA_sites_replaced=36,OTA_double_casts_replaced=2,OTA_reverse_transform_exact=True,unchanged_API_state_grouping_constants=True,conversion_edits=['retain upstream f64_to_f32 integer algorithm and full notice','replace includes by exact small representation types/macros','local flags context and reuse accepted static integer roundPack/jam helpers','fixed RNE/gradual/canonicalNaN policy inherited from accepted FMA32'],outputs={str(p.relative_to(R)):H(p.read_bytes())for p in[R/'candidate/OriginalOTAFilter.h',R/'baseline/OriginalOTA-FMA-only.h',R/'math/numeric_boundary.c']});(R/'DERIVATION.json').write_text(json.dumps(j,indent=2)+'\n');print(json.dumps(j,indent=2))
