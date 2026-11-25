"""Isolated native link. Disposable emulation inputs only; no installer/package path."""
import hashlib,os,re,struct,zlib
from pathlib import Path
from tools import build as b
real=b.tc
class Stop(Exception):pass
def seal(tool,*args):
    if tool!='pi32v2/bin/ld': return real(tool,*args)
    cmd=list(args)
    at=cmd.index('-T')+1
    script=Path(cmd[at]).read_text()
    out=Path(os.environ['D9_LAB_OUT'])/b.MODEL
    out.mkdir(parents=True,exist_ok=True)
    elf=out/'never-install.elf'; ld=out/'lab.ld'; mp=out/'lab.map'
    cmd[at]=ld;cmd[-1]=elf
    names=('crash','ride','hh')
    tables=''.join(' *(.rodata.x0x_smp_'+n+')' for n in names)
    syms=('text','ram_text','c1_text','data','xdata','lab_fdata')
    def link(header):
        h=' '.join('LONG(0x%08x);'%x for x in struct.unpack('<8I',header))
        s=script.replace('    XIP    (rx)','    LABFDATA (r) : ORIGIN = 0x020E1000, LENGTH = 0xF000\n    XIP    (rx)')
        s=s.replace('SECTIONS\n{\n','SECTIONS\n{\n .lab_fdata : { cold_lab_start = .; '+h+tables+' cold_lab_end = .; } > LABFDATA\n')
        s=s.replace(' *(.rodata*) . = ALIGN(4); _etext', ' *(.rodata*) . = ALIGN(4); cold_lab_expected = .; '+h+' . = ALIGN(4); _etext')
        assert s!=script and s.count('cold_lab_')==3
        ld.write_text(s)
        real(tool,*cmd,'-Map='+str(mp))
        for n in syms: real('common/bin/objcopy','-O','binary','-j','.'+n,elf,out/(n+'.bin'))
        return [(out/(n+'.bin')).read_bytes() for n in syms]
    a=link(bytes(32))
    p=a[-1][32:]
    assert len(a[-1])==55307 and a[-1][:32]==bytes(32)
    i={'elite':1,'pro':2}[b.MODEL]
    ident=hashlib.sha256(b'D9-READONLY-LAB-1'+bytes([i])+b''.join(struct.pack('<I',len(x))+x for x in (*a[:-1],p))).digest()[:12]
    h=struct.pack('<4s4I12s',b'D9RD',1,i,len(p),zlib.crc32(p),ident)
    z=link(h)
    n=z[0].index(h)
    assert z[0].count(h)==1 and z[0][:n]+bytes(32)+z[0][n+32:]==a[0] and z[1:-1]==a[1:-1] and z[-1]==h+p
    m=mp.read_text()
    src=(Path(b.OUT)/'gen/x0x_drum_samples.h').read_text()
    for name in names:
        addr,size=(int(x,16) for x in re.search(r'^ \.rodata\.x0x_smp_'+name+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M).groups())
        q=re.search(r'x0x_smp_'+name+r'\[\d+\] = \{([^}]+)\}',src)[1]
        data=bytes(int(v.strip(),0) for v in q.split(',') if v.strip())
        off=addr-0x020E1000
        assert off>=32 and len(data)==size and z[-1][off:off+size]==data
    used=sum(map(len,z[:4]))
    pool=int(re.search(r'^\.pool\s+0x[0-9a-f]+\s+0x([0-9a-f]+)',m,re.M)[1],16)
    print('guarded',b.MODEL,'XIP',used,'free',0x86ee0-used,'pool usable',0x54000-pool-8192,'sealed asset',len(z[-1]),'header',h.hex(),'ELF',hashlib.sha256(elf.read_bytes()).hexdigest(),flush=True)
    raise Stop
b.CFLAGS += ['-DX0X_COLD_COMPANION_LAB=1']
b.generate=lambda:None
b.build_loader=lambda:None
b.tc=seal
try:b.main()
except Stop:pass
