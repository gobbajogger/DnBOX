"""Authenticate defaults, musical dispatch, and final writable delay feedback."""
import runpy,struct,sys
W=runpy.run_path(sys.argv[2])
im,B,ins,call=W['im'],W['BASE'],W['ins'],W['callees']
for a,v in {0x100609818:.513,0x100609810:.863,0x100609808:1.726,
            0x100607a48:.202,0x100607a60:.505,0x100607a68:.555}.items():
    assert struct.unpack_from('<d',im.b,a-B)[0]==v
for a,v in {0x100612080:.2,0x100612914:.05}.items():
    assert im.b[a-B:a-B+4]==struct.pack('<f',v)
for a,w in {
 0x1000f3994:'mov w2, #1',0x1000f39bc:'mov w2, #2',0x1000f39dc:'mov w2, #3',
 0x1000f3a0c:'cmp w8, #3',0x1000f3a14:'csel x8, x8, xzr, eq',
 0x1000f3a64:'cbz w8, #476',0x1000f3b18:'mov w0, #42',
 0x1000f3bb0:'mov w0, #43',0x1000f3c34:'mov w0, #44',
 0x1003ad958:'tbz w8, #0, #712',0x1003ad95c:'ldr w8, [x19, #296]',
 0x1003ad9f4:'ldr w1, [x19, #316]',0x1003add58:'cmp w1, #2',
 0x1003adb10:'ldr s15, [x19, #208]',0x1003adb18:'fadd s0, s0, s15',
 0x1003adb28:'fadd s0, s0, s15',0x1003adb70:'fadd s0, s11, s0',
 0x1003adb88:'fmadd s1, s8, s13, s14',0x1003adb8c:'fmul s1, s15, s1',
 0x1003adb90:'fmadd s0, s0, s13, s14',0x1003adb94:'fmul s8, s15, s0',
 0x1003a9da8:'str s0, [x8, #16]',0x1003a9db4:'str s0, [x8, #64]',
 0x10037de74:'ldr s2, [x0, #16]',0x10037de78:'fmadd s0, s2, s1, s0'
}.items():ins(a,w)
assert call(0x1003adb20)==call(0x1003adba0)==0x1003a9da4
assert call(0x1003adb30)==call(0x1003adbac)==0x1003a9db0
assert call(0x1003adbc4)==0x1003a9dbc
assert call(0x1003a9f80)==call(0x1003a9fac)==0x10037de0c
print('PASS final feedback setters are multiplicative and supersede earlier same-slot mode offsets before render')
print('PASS exact public OFF/I/II/Both app reapplication and independent private alternate selector')
