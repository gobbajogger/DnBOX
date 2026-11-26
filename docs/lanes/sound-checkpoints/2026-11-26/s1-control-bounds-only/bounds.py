import hashlib,ctypes as c,pathlib, struct
p=pathlib.Path(__file__).parent; d=c.CDLL(str(p/'render.so'))
d.s1c_front_bounds.argtypes=[c.c_uint,c.POINTER(c.c_float),c.POINTER(c.c_float)]
d.s1c_front_init.restype=c.c_float; d.s1c_front_init.argtypes=[c.c_uint]
d.s1c_front_clamp.restype=c.c_float; d.s1c_front_clamp.argtypes=[c.c_uint,c.c_float]
h=hashlib.sha256()
for i in range(150):
 a,b=c.c_float(),c.c_float()
 d.s1c_front_bounds(i,c.byref(a),c.byref(b))
 xs=[-3.4e38,a.value,b.value,-0.,0.,.5,3.4e38]
 h.update(struct.pack('<10f',a.value,b.value,d.s1c_front_init(i),*(d.s1c_front_clamp(i,x) for x in xs)))
print(h.hexdigest(), '150 exact min/max and init domains, signed bounds')
