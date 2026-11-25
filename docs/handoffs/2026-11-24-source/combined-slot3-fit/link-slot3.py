"""Isolated native link. Disposable emulation inputs only; no installer/package path."""
import hashlib,os,re,struct,zlib
from pathlib import Path
from tools import build as b
real=b.tc
class Stop(Exception):pass
def seal(tool,*args):
    if tool!='pi32v2/bin/ld': return real(tool,*args)
    cmd=list(args)
    cmd.insert(cmd.index('--gc-sections'),'--relax')
    at=cmd.index('-T')+1
    script=Path(cmd[at]).read_text()
    out=Path(os.environ['D9_LAB_OUT'])/b.MODEL
    out.mkdir(parents=True,exist_ok=True)
    elf=out/'never-install.elf'; ld=out/'lab.ld'; mp=out/'lab.map'
    cmd[at]=ld;cmd[-1]=elf
    names=('crash','ride','hh')
    tables=''.join(' *(.rodata.x0x_smp_'+n+')' for n in names)
    relocated={'.rodata.x0x_tanh_raw':0x1008,
               '.rodata.s1_pan_detent_hz':0x200,
               '.rodata.s1_pan_boundary_hz':0x1fc,
               '.rodata.sj6_velocity_scale':0x200}
    tables+=''.join(' *('+n+')' for n in relocated)
    syms=('text','ram_text','c1_text','data','xdata','lab_fdata','lab_slot3')
    pair={
        '.rodata.original_chicago_packed_e_data':0x42b7,
        '.rodata.original_chicago_packed_a_data':0x1dc1,
    }
    held=''.join(' *('+n+')' for n in pair)
    def link(header,companion):
        h=' '.join('LONG(0x%08x);'%x for x in struct.unpack('<8I',header))
        h2=' '.join('LONG(0x%08x);'%x for x in struct.unpack('<8I',companion))
        s=script.replace('    XIP    (rx)','    LABFDATA (r) : ORIGIN = 0x020E1000, LENGTH = 0xF000\n    LABSLOT3 (r) : ORIGIN = 0x020AF000, LENGTH = 0x8000\n    XIP    (rx)')
        s=s.replace('SECTIONS\n{\n','SECTIONS\n{\n .lab_fdata : { cold_lab_start = .; '+h+tables+' cold_lab_end = .; } > LABFDATA\n .lab_slot3 : { cold_slot3_start = .; '+h2+held+' cold_slot3_end = .; } > LABSLOT3\n')
        s=s.replace(' *(.rodata*) . = ALIGN(4); _etext', ' *(.rodata*) . = ALIGN(4); cold_lab_expected = .; '+h+' cold_slot3_expected = .; '+h2+' . = ALIGN(4); _etext')
        assert s!=script and s.count('cold_lab_')==3 and s.count('cold_slot3_')==3
        ld.write_text(s)
        real(tool,*cmd,'-Map='+str(mp))
        for n in syms: real('common/bin/objcopy','-O','binary','-j','.'+n,elf,out/(n+'.bin'))
        return [(out/(n+'.bin')).read_bytes() for n in syms]
    a=link(bytes(32),bytes(32))
    p,p2=(a[-2][32:],a[-1][32:])
    assert 55307+sum(relocated.values())<=len(a[-2])<=61440
    assert 32+sum(pair.values())<=len(a[-1])<=32768
    assert a[-2][:32]==a[-1][:32]==bytes(32)
    i={'elite':1,'pro':2}[b.MODEL]
    ident=hashlib.sha256(b'D9-READONLY-LAB-2'+bytes([i])+b''.join(struct.pack('<I',len(x))+x for x in (*a[:-2],p,p2))).digest()[:12]
    h=struct.pack('<4s4I12s',b'D9RD',2,i,len(p),zlib.crc32(p),ident)
    h2=struct.pack('<4s4I12s',b'D9T3',2,i,len(p2),zlib.crc32(p2),ident)
    z=link(h,h2)
    n=z[0].index(h)
    assert z[0].count(h)==z[0].count(h2)==1 and z[0].index(h2)==n+32
    assert z[0][:n]+bytes(64)+z[0][n+64:]==a[0] and z[1:-2]==a[1:-2]
    assert z[-2]==h+p and z[-1]==h2+p2
    m=mp.read_text()
    for name,expected in relocated.items():
        aa,sz=(int(x,16) for x in re.search(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M).groups())
        assert sz==expected and 0x020E1000+55307<=aa and aa+sz<=0x020E1000+len(z[-2]),(name,aa,sz)
    for name,expected in pair.items():
        aa,sz=(int(x,16) for x in re.search(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M).groups())
        assert sz==expected and 0x020AF000+32<=aa and aa+sz<=0x020AF000+len(z[-1]),(name,aa,sz)
    src=(Path(b.OUT)/'gen/x0x_drum_samples.h').read_text()
    for name in names:
        addr,size=(int(x,16) for x in re.search(r'^ \.rodata\.x0x_smp_'+name+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M).groups())
        q=re.search(r'x0x_smp_'+name+r'\[\d+\] = \{([^}]+)\}',src)[1]
        data=bytes(int(v.strip(),0) for v in q.split(',') if v.strip())
        off=addr-0x020E1000
        assert off>=32 and len(data)==size and z[-2][off:off+size]==data
    used=sum(map(len,z[:4]))
    pool=int(re.search(r'^\.pool\s+0x[0-9a-f]+\s+0x([0-9a-f]+)',m,re.M)[1],16)
    print('guarded',b.MODEL,'XIP',used,'free',0x86ee0-used,'pool usable',0x54000-pool-8192,'read-only FDATA',len(z[-2]),'diagnostic slot3 span',len(z[-1]),'slot3 free',32768-len(z[-1]),'header',h.hex(),'ELF',hashlib.sha256(elf.read_bytes()).hexdigest(),flush=True)
    raise Stop
b.CFLAGS += ['-DX0X_COLD_COMPANION_LAB=1','-DX0X_COLD_SLOT3_LAB=1']
b.CFLAGS.extend(["-DX0X_S1_FEEDBACK_PROBE=1","-DX0X_S1_DELAY_TRACK_TEST=1","-DX0X_SJ6_FILTER_VEL_PROBE=1","-DX0X_SJ6_VELOCITY_PORT=1","-DX0X_SJ6_NATIVE_RANDOM2=1"])
b.CFLAGS.append("-DX0X_SJ6_NATIVE_POSITIVE_GATE=1")
b.CFLAGS += ["-DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1","-DX0X_S1_MOD_RATE_PROBE=1","-DX0X_S1_BEND_RANGE_PROBE=1","-DX0X_S1_AUTOPAN_PROBE=1","-DX0X_S1_AUTOPAN_PORT=1","-DX0X_SJ6_MONO32_PROBE=1"]
b.CFLAGS.extend(["-DX0X_S1_FEEDBACK_PROBE=1","-DX0X_S1_DELAY_TRACK_TEST=1","-DX0X_SJ6_FILTER_VEL_PROBE=1","-DX0X_SJ6_VELOCITY_PORT=1","-DX0X_SJ6_NATIVE_RANDOM2=1"])
b.CFLAGS.append("-DX0X_SJ6_NATIVE_POSITIVE_GATE=1")
b.ORIGINAL_SYNTH_OZ_UNITS |= {'chicago_selector','chicago_arp_control','chicago_note_context','miami_geometry','miami_fx_conversion'}
b.generate=lambda:None
b.build_loader=lambda:None
b.tc=seal
try:b.main()
except Stop:pass
