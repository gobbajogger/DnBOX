from static import *
targets={int(x,0) for x in sys.argv[1:]};v,n,o=textseg
for i,(w,) in enumerate(struct.iter_unpack('<I',b[o:o+n])):
 if w&0xfc000000==0x94000000:
  imm=w&0x3ffffff;imm=imm-(1<<26) if imm&(1<<25) else imm;a=v+4*i;t=a+4*imm
  if t in targets:print(hex(a),hex(func(a)),hex(t))
