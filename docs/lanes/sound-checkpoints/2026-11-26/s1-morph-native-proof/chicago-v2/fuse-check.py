from pathlib import Path
import sys
p=Path(sys.argv[1])/'tree/experiments/original_synth/chicago_mp4.c'
t=p.read_text()
a='/* One Chicago voice sample: oscillators (sync 1), noise, MP4 filter and'
b='''/* IEEE binary32: abs(Inf/NaN) alone reaches 0x7f800000. The following
 * unsigned addition sets bit31 exactly for that subset. Test five values
 * with one final branch, preserving the full state-commit guard. */
static inline uint32_t ck_bad32(float v)
{
 return (original_math_bits(v) & 0x7fffffffu)+0x00800000u;
}

'''
# Apply only the two very hot fused voice loops, not the public per-frame DSP
s=t.index(a); u=t[:s]+b; t=t[s:]
a=' if(!original_binary32_isfinite(z1)||!original_binary32_isfinite(z2)||!original_binary32_isfinite(z3)||!original_binary32_isfinite(z4)||!original_binary32_isfinite(tmp))return 0;'
b=' if((ck_bad32(z1)|ck_bad32(z2)|ck_bad32(z3)|ck_bad32(z4)|ck_bad32(tmp))>>31)return 0;'
assert t.count(a)==2
p.write_text(u+t.replace(a,b))
