"""Isolated native link. Disposable emulation inputs only; no installer/package path."""
import hashlib,os,re,struct,zlib
from pathlib import Path
from tools import build as b
real=b.tc
class Stop(Exception):pass
def seal(tool,*args):
    if tool!='pi32v2/bin/ld': return real(tool,*args)
    if os.environ.get('DNBOX_CONTROL_BUILD_ONLY')=='1':
        print('compiled-controls',b.MODEL,flush=True)
        raise Stop
    if os.environ.get('DNBOX_CONTROL_BUILD_ONLY')=='1':
        # Every translation unit was compiled with the actual model flags.
        # Do not invoke the linker, report size, package, or run firmware.
        print('compiled-controls',b.MODEL,flush=True)
        raise Stop
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
    assert b.synth_controls_init_only(), 'FITTAB lab is only allowed with factory banks absent'
    extra_fdata={
        '.rodata.APC40_PAD_OWNER': (384,'e39797151468677c6e5735ca61944df77925e46cf7cfe30ff3ab687181795de7'),
    }
    exact_fittab={
        '.rodata.original_miami_n513_bits': (2052,'4880416fdae25c3fef0c3885c358f65a5f42d4470f0f6e7d8d8922a115de800c'),
        '.rodata.original_chicago_packed_e_offset': (1026,'b4c167230d8964dc8c0a06c6e1681b7a98768822bc8abee009f78bb1e8dbe823'),
        '.rodata.compact_miami_factory_codes': (480,'6393cd72abdd5e07b0dff12191907d89857e00db50fecb981ac62c1081e99f67'),
        '.rodata.compact_miami_names': (339,'43d7f6dad8ad1bd8b14630dc857dc6bd1ff1ccf3047ac9f23d271917a9cfaf80'),
    }
    tables+=''.join(' *('+n+')' for n in (*relocated,*extra_fdata))
    ft=''.join(' *('+n+')' for n in exact_fittab)
    syms=('text','ram_text','c1_text','data','lab_fittab','xdata','lab_fdata','lab_slot3')
    generated=Path(b.OUT)/'gen/chicago_compact_private.h'
    raw=generated.read_bytes()
    # This forward source has the intact original compiler-generated format,
    # rather than the later compressed payload bytes lost with the prior runner.
    assert hashlib.sha256(raw).hexdigest()=='22beb20b008a93a22000752e62b25018b793bae5f69b1aca5d06c8152916e189'
    vals={}
    for name in ('original_chicago_packed_e_data','original_chicago_packed_a_data'):
        count,body=re.search(r'uint8_t '+name+r'\[(\d+)\]\s*=\s*\{([^}]+)\}',raw.decode()).groups()
        data=bytes(int(x.strip(),0) for x in body.split(',') if x.strip())
        assert len(data)==int(count)
        vals['.rodata.'+name]=data
    pair={k:len(v) for k,v in vals.items()}
    # Cold immutable stock program metadata, byte-identical to same-build
    # APP references on both models. No payload fixtures or slot writes.
    cold_factory={
        '.rodata.original_chicago_factory_codes': (2005,'c4f2c076f7cd43735bdd0ade4265f89af86b7ceeac62f9e1d0946c032a21c9eb'),
        '.rodata.original_chicago_factory_dictionary': (1236,'87921e3521e84543e7f0224a804139b6ddd57c5b8a5decdaa89e75d2f12b3577'),
        '.rodata.original_chicago_factory_names': (879,'0283818ed72be42cab3ca326e1631fb231dd1bdb9343ac3e65ccd5cb49d5a281'),
        '.rodata.original_program_value_words': (908,'5483735b4f2274809b5acd3b80b00bc1a7670e7cf04579faec7354c63bb9f668'),
    }
    cold_factory.update({
        '.rodata.original_chicago_packed_a_offset': (1026,'080f99526318fbc82901d2dd87bdecd618ada4ee33bb642d6296df6c2917183e'),
        '.rodata.original_chicago_eq_amplitude': (512,'be9953b585250950df04653089e243d84869ba2e4a711ea46494e5a528dcccc2'),
    })
    held=''.join(' *('+n+')' for n in (*pair,*cold_factory))
    def link(header,companion,fittab):
        h=' '.join('LONG(0x%08x);'%x for x in struct.unpack('<8I',header))
        h2=' '.join('LONG(0x%08x);'%x for x in struct.unpack('<8I',companion))
        h3=' '.join('LONG(0x%08x);'%x for x in struct.unpack('<8I',fittab))
        s=script.replace('    XIP    (rx)','    LABFDATA (r) : ORIGIN = 0x020E1000, LENGTH = 0xF000\n    LABSLOT3 (r) : ORIGIN = 0x020AF000, LENGTH = 0x8000\n    LABFITTAB (r) : ORIGIN = 0x020FA000, LENGTH = 0x1000\n    XIP    (rx)')
        s=s.replace('SECTIONS\n{\n','SECTIONS\n{\n .lab_fdata : { cold_lab_start = .; '+h+tables+' cold_lab_end = .; } > LABFDATA\n .lab_slot3 : { cold_slot3_start = .; '+h2+held+' cold_slot3_end = .; } > LABSLOT3\n .lab_fittab : { cold_fittab_start = .; '+h3+ft+' cold_fittab_end = .; } > LABFITTAB\n')
        s=s.replace(' *(.rodata*) . = ALIGN(4); _etext', ' *(.rodata*) . = ALIGN(4); cold_lab_expected = .; '+h+' cold_slot3_expected = .; '+h2+' cold_fittab_expected = .; '+h3+' . = ALIGN(4); _etext')
        assert s!=script and s.count('cold_lab_')==3 and s.count('cold_slot3_')==3 and s.count('cold_fittab_')==3
        ld.write_text(s)
        real(tool,*cmd,'-Map='+str(mp))
        for n in syms: real('common/bin/objcopy','-O','binary','-j','.'+n,elf,out/(n+'.bin'))
        return [(out/(n+'.bin')).read_bytes() for n in syms]
    a=link(bytes(32),bytes(32),bytes(32))
    p,p2,p3=(a[-2][32:],a[-1][32:],a[4][32:])
    assert 55307+sum(relocated.values())<=len(a[-2])<=61440
    assert 32+sum(pair.values())<=len(a[-1])<=32768
    assert 32+sum(v[0] for v in exact_fittab.values())<=len(a[4])<=4096
    assert a[-2][:32]==a[-1][:32]==a[4][:32]==bytes(32)
    i={'elite':1,'pro':2}[b.MODEL]
    ident=hashlib.sha256(b'D9-READONLY-LAB-3'+bytes([i])+b''.join(struct.pack('<I',len(x))+x for x in (*a[:4],a[5],p,p2,p3))).digest()[:12]
    h=struct.pack('<4s4I12s',b'D9RD',2,i,len(p),zlib.crc32(p),ident)
    h2=struct.pack('<4s4I12s',b'D9T3',2,i,len(p2),zlib.crc32(p2),ident)
    h3=struct.pack('<4s4I12s',b'D9FT',2,i,len(p3),zlib.crc32(p3),ident)
    z=link(h,h2,h3)
    n=z[0].index(h)
    assert z[0].count(h)==z[0].count(h2)==1 and z[0].index(h2)==n+32
    assert z[0].count(h3)==1 and z[0].index(h3)==n+64
    assert z[0][:n]+bytes(96)+z[0][n+96:]==a[0] and z[1:4]==a[1:4] and z[5]==a[5]
    assert z[4]==h3+p3
    assert z[-2]==h+p and z[-1]==h2+p2
    m=mp.read_text()
    for name,expected in relocated.items():
        aa,sz=(int(x,16) for x in re.search(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M).groups())
        assert sz==expected and 0x020E1000+55307<=aa and aa+sz<=0x020E1000+len(z[-2]),(name,aa,sz)
    for name,expected in pair.items():
        aa,sz=(int(x,16) for x in re.search(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M).groups())
        assert sz==expected and 0x020AF000+32<=aa and aa+sz<=0x020AF000+len(z[-1]),(name,aa,sz)
        off=aa-0x020AF000
        assert z[-1][off:off+sz]==vals[name],name
    for name,(expected,digest) in cold_factory.items():
        entries=re.findall(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M)
        assert len(entries)==1,(name,entries)
        addr,size=map(lambda x:int(x,16),entries[0])
        off=addr-0x020AF000
        assert size==expected and off>=32 and off+size<=len(z[-1]),(name,addr,size)
        assert hashlib.sha256(z[-1][off:off+size]).hexdigest()==digest,name
    for base,image,expect in ((0x020FA000,z[4],exact_fittab),(0x020E1000,z[-2],extra_fdata)):
        for name,(size,digest) in expect.items():
            entries=re.findall(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M)
            assert len(entries)==1,(name,entries)
            addr,count=map(lambda x:int(x,16),entries[0]); offset=addr-base
            assert count==size and 32<=offset<=len(image)-count,(name,addr,count)
            assert hashlib.sha256(image[offset:offset+count]).hexdigest()==digest,name
    src=(Path(b.OUT)/'gen/x0x_drum_samples.h').read_text()
    for name in names:
        addr,size=(int(x,16) for x in re.search(r'^ \.rodata\.x0x_smp_'+name+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M).groups())
        q=re.search(r'x0x_smp_'+name+r'\[\d+\] = \{([^}]+)\}',src)[1]
        data=bytes(int(v.strip(),0) for v in q.split(',') if v.strip())
        off=addr-0x020E1000
        assert off>=32 and len(data)==size and z[-2][off:off+size]==data
    # Use the stock packer's load offsets, inter-section gaps and final marker,
    # not merely the sum of the ELFs' extracted section lengths.
    syms=real('common/bin/objdump','-t',elf)
    end=len(z[0])
    for data,name in zip(z[1:4],('_rt_load','_c1_load','_data_load')):
        if not data: continue
        load=int(re.search(r'^([0-9a-f]+) .*\s'+name+r'$',syms,re.M)[1],16)
        offset=load-b.APP_XIP
        assert load%4==0 and offset>=end,(name,offset,end)
        end=offset+len(data)
    used=((end+3)&~3)+4
    pool=int(re.search(r'^\.pool\s+0x[0-9a-f]+\s+0x([0-9a-f]+)',m,re.M)[1],16)
    print('guarded',b.MODEL,'XIP',used,'free',0x86ee0-used,'pool usable',0x54000-pool-8192,'read-only FDATA',len(z[-2]),'diagnostic slot3 span',len(z[-1]),'slot3 free',32768-len(z[-1]),'read-only FITTAB',len(z[4]),'header',h.hex(),'ELF',hashlib.sha256(elf.read_bytes()).hexdigest(),flush=True)
    raise Stop
b.CFLAGS += ['-DX0X_COLD_COMPANION_LAB=1','-DX0X_COLD_SLOT3_LAB=1','-DX0X_COLD_FITTAB_LAB=1']
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
