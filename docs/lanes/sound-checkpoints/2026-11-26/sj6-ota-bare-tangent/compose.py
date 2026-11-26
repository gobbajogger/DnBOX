from pathlib import Path
import sys,re
base,wire,out=map(Path,sys.argv[1:])
s=base.read_text()
t=wire.read_text().split("s[:lo]+'''",1)[1].split("'''+s[hi:]",1)[0]
t=t.replace("void prepare_tile","static inline void prepare_tile")
t=t.replace("fmaf(","split_mul_add(")
lhs=s.index("static float clock_2x"); rhs=s.index("static inline float physical_tick",lhs)
s=s[:lhs]+t+s[rhs:]
s=s.replace("    const float g=params[0];\n    const float gz=g/(1.f+g);\n    const float gz_inverse=1.f-gz;\n","")
assert s.count("coeff,g,params[1],params[2],params[3],gz,gz_inverse")==2
s=s.replace("coeff,g,params[1],params[2],params[3],gz,gz_inverse","coeff,params")
out.write_text(s)
