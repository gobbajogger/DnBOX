import ctypes as c, itertools, json, pathlib, sys, time
p=pathlib.Path(__file__).parent
f=c.CDLL(str(p/'render.so')).fenv_witness; f.restype=c.c_uint64
cases={}
for ft,r,n in itertools.product(range(3),range(4),(1,4)):
 t=time.perf_counter();h=f(ft,r,n,8);elapsed=time.perf_counter()-t
 assert h
 cases[f'{ft}/{r}/{n}']=[h,elapsed]
print(json.dumps(cases))
