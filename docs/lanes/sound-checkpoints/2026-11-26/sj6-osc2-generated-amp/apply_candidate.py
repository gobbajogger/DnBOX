"""Apply the guarded modern generated-OSC2 gain proof to an isolated donor."""
import argparse
import hashlib
from pathlib import Path

PRIOR = "bd5a59c1b46fdf65f7d4e8eb0d9780fcbf21841906d00a672309a4ee20532f76"
AFTER = "16cce5cd0deec6c393c06935b4939bb4a2163efa25b100f318f8e68b157088bc"
OLD = b"                        const float voice_scale=(t*.5f)*.68f;"
NEW = b"""                        const float voice_scale=
#if DNBX_HOST_SJ6_CURRENT_OSC2_GENERATED_AMP
                            /* Exact modern generated OSC2 profile, not the
                             * legacy analytic/premultiplied owner. */
                            l==1u && !s->main_oracle.ablate_scale_inside_halfband ? t :
#endif
                            (t*.5f)*.68f;"""

def apply(source):
    if hashlib.sha256(source).hexdigest()!=PRIOR or source.count(OLD)!=1:
        raise ValueError("wrong recorded donor; do not transplant without review")
    p=source.index(OLD)
    context=source[max(0,p-1800):p+600]
    for sentinel in (b"source_amp_before_history",b"const float t=amp*(s->main_oracle.source_gain[l]*vel);",b"const float generated_profile=1.5309427976608276f;",b"ablate_scale_inside_halfband"):
        if sentinel not in context:
            raise ValueError("generated route context drift")
    candidate=source.replace(OLD,NEW,1)
    if hashlib.sha256(candidate).hexdigest()!=AFTER:
        raise ValueError("patched source differs from the independently compiled receipt")
    return candidate

if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("donor",type=Path)
    p.add_argument("candidate",type=Path)
    a=p.parse_args()
    patched=apply(a.donor.read_bytes())
    with a.candidate.open("xb") as f:
        f.write(patched)
    print(hashlib.sha256(patched).hexdigest())
