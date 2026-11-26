"""Isolated native link. Disposable emulation inputs only; no installer/package path."""
import hashlib,os,re,struct,zlib
from pathlib import Path
from tools import build as b
assert os.environ.get("D9_LAB_XDATA_TAIL")=="1", "XDATA-tail laboratory must be explicitly enabled"
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
    names=('crash','ride','hh','huff_lut')
    tables=''.join(' *(.rodata.x0x_smp_'+n+')' for n in names)
    relocated={'.rodata.x0x_tanh_raw':0x1008,
               '.rodata.s1_pan_detent_hz':0x200,
               '.rodata.s1_pan_boundary_hz':0x1fc,
               '.rodata.sj6_velocity_scale':0x200}
    split = os.environ.get('DNBOX_TANH_SPLIT_LAB') == '1'
    if split:
        relocated.pop('.rodata.x0x_tanh_raw')
        relocated.update({'.rodata.x0x_tanh_lab_head':1600,'.rodata.x0x_tanh_lab_tail':1252})
    assert b.synth_controls_init_only(), 'FITTAB lab is only allowed with factory banks absent'
    extra_fdata={
        '.rodata.APC40_PAD_OWNER': (384,'e39797151468677c6e5735ca61944df77925e46cf7cfe30ff3ab687181795de7'),
    }
    if split:
        extra_fdata.update({
            '.rodata.x0x_tanh_lab_head':(1600,'9cce98ce40e37b6b28e389229844eca61b153116956a1d9afc350f5c5c4defea'),
            '.rodata.x0x_tanh_lab_tail':(1252,'6f4f3d420d74dc070615a53c0aeca829dbe778619041ac122f9b12872a5f9a27'),
        })
    if os.environ.get("DNBOX_TANH_REFILL_LAB") == "1":
        assert split, "LCD refill requires space created by guarded split tanh"
        extra_fdata.update({
            '.rodata.arc8_turns': (504,'7d880cfad5a08b1760af9a3f67a03229309b03c1c7af0c85174923f1a8b77d3a'),
            '.rodata.arc8_pixels': (252,'8b06d9f6f7e16d0166044f12ca428ddef76dfe07919c24c5d6177ba6d8a31f09'),
            '.rodata.APC40_LCD565': (256,'e6f130e761b3873e33657f9c5ef885d4530c51b4f95e8865601f911921c614e3'),
            '.rodata.CORNER_PACKED': (204,'532a5b143a9620ac6ac6b67372bc9ae48cb76d569dc7a033b6b3a87c45c9c339'),
        })
    extra_fdata.update({
        '.rodata.DV_XY_PACKED': (1385, 'b85232afb0bc55f0b01d3785f2930de3a673728118120fecaabc0a402351037f'),
        '.rodata.DV_POLY_PACKED': (243, '5da4d1b265a7d90cb991cc65ea4db116fbf0dbe41ae32c4b871fa9aead369596'),
        '.rodata.UI_PICTOGRAM_LOW': (333, '438de1bb3b27ed49f6f2f76c21d3b8ec009690163e5fd3c7faa33d56be077d00'),
        '.rodata.UI_PICTOGRAM_HIGH': (42, '4a89d56faa589463ada908d4b98bb40093ec8399a34f8f1c386134c8577209bc'),
        '.rodata.brk_retrigger_groups': (975, '7a6cd0bcb6560b738184b183ecc9d183ca3110308a3f135decaab79b4b03afdc'),
        '.rodata.original_chicago_eq_sin': (512, '9656f4300eb60486c3542b370804dfe85fd7bc749f386db50c1afed5d86b8ff9'),
        '.rodata.original_chicago_eq_cos': (512, '09e31976123fe609ffc6da59f59318694d8bafeb66c1d4e05fe78f4be173675c'),
    })
    s1_fdata={
        '.rodata.patch_domain': (144,'1850293ba738017d5c152031d40c0f33ce3f2204b47c7784e56d42eb75100da1'),
        '.rodata.patch_slot_code': (150,'2ee5a02b37043b1bbffa6f6cef1e81efc7960620ef0e9297a8a63affe4aa7e42'),
    } if os.environ.get('D9_LAB_S1_FDATA')=='1' else {}
    if os.environ.get('D9_LAB_SPARE_TABLES')=='1':
        s1_fdata['.rodata.original_chicago_lookup']=(168, '74ccae1e0175f4ae0880d03045afad2aab526d1c4a1b00f452f2caecee0b3678')
    extra_fdata.update(s1_fdata)
    streams={
        'crash':(19851,'c8b8b64a14351a07f4a4b4d2fdc340a5674e5d1d20be1b4c91fd8d3fd1b0ad22'),
        'ride':(21746,'6be8363921814de87fb5037f3604634ef68064f0f8a60226f7f0ffa56f5113b3'),
        'hh':(9286,'36877ea6c212f0741faafadb29e9da7133684723315770fb81fbc33f373c9c3d'),
        'huff_lut':(64,'3f72e3302a803c88212bf4babde1d93833f39c39695aee3e33bc97b2af744a19'),
    }
    cold909_end=32+sum(n for n,_ in streams.values())
    exact_fittab={
        '.rodata.sin_q': (260,'a8156345a2fb982e49e5ffdffcb65c88f2f75d0b39d72a6263e6a59ee62a6484'),
        '.rodata.original_sound_upper_value': (256,'cf0919cc8b865aef649cf1b20dceb5f734742977c831e4f7102ef59610d30d71'),
        '.rodata.sj6_midi_velocity': (384,'5c5d2e91d4b58ad5989a770fa9d69175a8f630f0cff793366d869e7d8e8c53ae'),
        ".rodata.s1c_iinit": (150,"cbb81dd5ecfee5c26520ccc38241c6e6bd7cd79ea2b9b50bd213c49710904a4e"),
        '.rodata.original_miami_n513_bits': (2052,'4880416fdae25c3fef0c3885c358f65a5f42d4470f0f6e7d8d8922a115de800c'),
        '.rodata.compact_miami_factory_codes': (480,'6393cd72abdd5e07b0dff12191907d89857e00db50fecb981ac62c1081e99f67'),
        '.rodata.compact_miami_names': (339,'43d7f6dad8ad1bd8b14630dc857dc6bd1ff1ccf3047ac9f23d271917a9cfaf80'),
    }
    if os.environ.get('D9_LAB_SPARE_TABLES')=='1':
        exact_fittab['.rodata.compact_chicago_base_codes']=(117, '92d8f98eb9012d87537b0e2b645e377221b869f884314dfeb52593ebd3fddb9f')
    tables+=''.join(' *('+n+')' for n in dict.fromkeys((*relocated,*extra_fdata)))
    ft=''.join(' *('+n+')' for n in exact_fittab)
    syms=('text','ram_text','c1_text','data','lab_fittab','xdata','lab_fdata','lab_slot3')
    vals={}
    for tag,n,sha in (('a',6099,'e60b242e17be038dfb34d9c9c046cbae87e88299b7eb5658beb7b0be20944a44'),('e',15629,'a907757d337ab56d9537509d1f86d8de64958bea5d33a98c442dcc2ed43535b8')):
        raw=(Path(b.OUT)/'gen'/('p_'+tag+'.h')).read_text()
        data=bytes(int(v.strip(),0) for v in re.search(r'\[\]\s*=\s*\{([^}]+)\}',raw)[1].split(',') if v.strip())
        assert len(data)==n and hashlib.sha256(data).hexdigest()==sha,tag
        vals['.rodata.p_'+tag]=data
    pair={k:len(v) for k,v in vals.items()}
    # Cold immutable stock program metadata, byte-identical to same-build
    # APP references on both models. No payload fixtures or slot writes.
    cold_factory={
        '.rodata.original_miami_a44100': (516,'3da0159419c20886468cf171c20772027aec755e39ec207fb06b9fd83363bdd9'),
        '.rodata.original_miami_b44100': (516,'59474b1aa179c1062e4ba342b10c3a97bbe3337e6c135639506a2df9818a8738'),
        '.rodata.original_pitch44100': (512,'77c39cdfb56f1cd4da2bfbe83d1c7902a0a8078e0b66f98045a49d0e9dc20926'),
        '.rodata.original_drive_gain_table': (256,'1379e27e9e32883c15cfb944ce8114f3d0f45b1dee71f2e255b20db3b54a3d2f'),
        ".rodata.s1_scale_ratio": (360,"fcf13207a079d2bad1fd2df5d93a296ff5625ab37b8a12dd9ce81839d74bbbc7"),
        ".rodata.s1_scale_names": (113,"17fbf474592eee7619571810c7e7e502ab9f0dcd46de93f18d641b91b1262abc"),
        '.rodata.original_chicago_factory_codes': (2005,'c4f2c076f7cd43735bdd0ade4265f89af86b7ceeac62f9e1d0946c032a21c9eb'),
        '.rodata.original_chicago_factory_dictionary': (1236,'87921e3521e84543e7f0224a804139b6ddd57c5b8a5decdaa89e75d2f12b3577'),
        '.rodata.original_chicago_factory_names': (879,'0283818ed72be42cab3ca326e1631fb231dd1bdb9343ac3e65ccd5cb49d5a281'),
        '.rodata.original_program_value_words': (908,'5483735b4f2274809b5acd3b80b00bc1a7670e7cf04579faec7354c63bb9f668'),
    }
    cold_factory.update({
        '.rodata.original_chicago_eq_amplitude': (512,'be9953b585250950df04653089e243d84869ba2e4a711ea46494e5a528dcccc2'),
    })
    slot3_metadata={
        '.rodata.original_miami_mappings':480,
        '.rodata.original_chicago_mappings':464,
        '.rodata.sj6_knob_desc.knobs':504,
        '.rodata.original_sound_juno_params':216,
        '.rodata.patch_parameters':496,
        '.rodata.sj6_aux_route_labels':276,
        '.rodata.rd_panel_sj6_ref.fixed':312,
        '.rodata.page_titles':344,
    } if os.environ.get("D9_LAB_SLOT3_METADATA")=="1" else {}
    if os.environ.get('D9_LAB_SPARE_TABLES')=='1':
        assert slot3_metadata
        slot3_metadata['.rodata.d9_cp_p']=96
    # These are linker input sections, not reserialized structs. PIE/absolute
    # internal references keep the native linker relocation records and actual
    # referenced output symbols. Linked bytes join the existing whole-companion seal.
    held=''.join(' *('+n+')' for n in (*pair,*cold_factory,*slot3_metadata))
    def audit_meta(image=None, blank=None):
        if not slot3_metadata:return
        m=mp.read_text()
        match=re.search(r'^\.lab_slot3\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M)
        assert match
        base,sz=map(lambda x:int(x,16),match.groups())
        assert base==0x020af000 and 32<=sz<=0x8000
        begin=base+32
        seen=[]
        for name,expected in slot3_metadata.items():
            entries=re.findall(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M)
            assert len(entries)==1,(name,entries)
            addr,n=map(lambda x:int(x,16),entries[0]); off=addr-base
            assert n==expected and begin<=addr and addr+n<=base+sz,(name,addr,n)
            seen.append((addr,addr+n))
            if image is not None:
                assert len(image)==len(blank)==sz
                assert image[off:off+n]==blank[off:off+n]
                print('slot3-metadata linked-sha256',b.MODEL,name,n,
                      hashlib.sha256(image[off:off+n]).hexdigest(),flush=True)
            else:
                print('slot3-metadata map-only',b.MODEL,name,hex(addr),n,flush=True)
        for (_,end),(next_start,_) in zip(sorted(seen),sorted(seen)[1:]):
            assert end<=next_start
        print('slot3-metadata span',b.MODEL,sz,'remaining',0x8000-sz,flush=True)
    xdata_tables = {
        '.rodata.original_sound_chicago_params': 264,
        '.rodata.original_sound_miami_params': 216,
    }
    def audit_fdata_map():
        if not s1_fdata:return
        mm=mp.read_text()
        rr=re.search(r'^\.lab_fdata\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',mm,re.M)
        assert rr
        base,size=(int(v,16) for v in rr.groups())
        assert base==0x020e1000 and cold909_end<size<=0xf000
        regions=[]
        for name,(size_expected,_) in s1_fdata.items():
            matches=re.findall(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',mm,re.M)
            assert len(matches)==1
            addr,n=map(lambda v:int(v,16),matches[0])
            assert n==size_expected and base+cold909_end<=addr<=base+size-n
            regions.append((addr,addr+n))
            print('FDATA S1 map-only',b.MODEL,name,hex(addr),n,flush=True)
        assert all(b<=c for (_,b),(c,_) in zip(sorted(regions),sorted(regions)[1:]))
        print('FDATA S1 tail span',b.MODEL,size,'remaining',0xf000-size,flush=True)
    def audit_fittab_map():
        if os.environ.get('D9_LAB_SPARE_TABLES')!='1':return
        mm=mp.read_text()
        rr=re.search(r'^\.lab_fittab\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',mm,re.M)
        assert rr
        base,size=(int(v,16) for v in rr.groups())
        assert base==0x020fa000 and 32<size<=0x1000
        seen=[]
        for name,(expected,_) in exact_fittab.items():
            hits=re.findall(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',mm,re.M)
            assert len(hits)==1
            addr,n=map(lambda v:int(v,16),hits[0])
            assert n==expected and base+32<=addr<=base+size-n
            seen.append((addr,addr+n))
            if name=='.rodata.compact_chicago_base_codes':
                print('FITTAB cold map-only',b.MODEL,name,hex(addr),n,flush=True)
        assert all(b<=c for (_,b),(c,_) in zip(sorted(seen),sorted(seen)[1:]))
        print('FITTAB span',b.MODEL,size,'remaining',0x1000-size,flush=True)
    def link(header,companion,fittab,xcrc):

        h=' '.join('LONG(0x%08x);'%x for x in struct.unpack('<8I',header))
        h2=' '.join('LONG(0x%08x);'%x for x in struct.unpack('<8I',companion))
        h3=' '.join('LONG(0x%08x);'%x for x in struct.unpack('<8I',fittab))
        s=script.replace('    XIP    (rx)','    LABFDATA (r) : ORIGIN = 0x020E1000, LENGTH = 0xF000\n    LABSLOT3 (r) : ORIGIN = 0x020AF000, LENGTH = 0x8000\n    LABFITTAB (r) : ORIGIN = 0x020FA000, LENGTH = 0x1000\n    XIP    (rx)')
        s=s.replace('    LABFDATA', '    LABMORPH (rx) : ORIGIN = 0x020CD000, LENGTH = 0x5000\n    LABFDATA',1)
        s=s.replace('SECTIONS\n{\n','SECTIONS\n{\n .lab_morph : { *(.rodata.morph_rows) *(.rodata.morph_bytes) *(.rodata.prep_morph.limits) *(.text.u16) *(.text.s16) *(.text.stored_lattice) *(.text.fold_index) *(.text.zig) *(.text.cubic_at) *(.text.stored_pair) *(.text.shared_shape) *(.text.morph_held) *(.text.prep_morph) *(.text.morph) } > LABMORPH\n',1)
        s=s.replace('SECTIONS\n{\n','SECTIONS\n{\n .lab_fdata : { cold_lab_start = .; '+h+tables+' cold_lab_end = .; } > LABFDATA\n .lab_slot3 : { cold_slot3_start = .; '+h2+held+' cold_slot3_end = .; } > LABSLOT3\n .lab_fittab : { cold_fittab_start = .; '+h3+ft+' cold_fittab_end = .; } > LABFITTAB\n')
        assert s.count('_xdata_end = .; } > XDATA') == 1
        tail=' . = ALIGN(4); cold_xdata_tail_start = .; '+''.join(' KEEP(*('+n+'))' for n in xdata_tables)+' cold_xdata_tail_end = .;'
        s=s.replace('_xdata_end = .; } > XDATA','_xdata_end = .;'+tail+' } > XDATA',1)
        # GNU ld assigns input sections in script order. The exact cold
        # selectors must run before .text's generic .rodata* collector.
        import re
        xm=re.search(r'^ +\.xdata.*?_xdata_end = \.;.*?} > XDATA[ 	]*$',s,re.M|re.S)
        assert xm and s.count(' .text :')==1
        xblock=xm.group(0)
        s=(s[:xm.start()]+s[xm.end():]).replace(' .text :',xblock+chr(10)+' .text :',1)
        s=s.replace(' *(.rodata*) . = ALIGN(4); _etext', ' *(.rodata*) . = ALIGN(4); cold_lab_expected = .; '+h+' cold_slot3_expected = .; '+h2+' cold_fittab_expected = .; '+h3+' cold_xdata_tail_crc = .; LONG(0x%08x); . = ALIGN(4); _etext'%xcrc)
        # Both arms consume the exact same separately emitted input function.
        if os.environ.get("FETCH_MODE")=="ram":
            assert s.count('KEEP(*(.ram_text*))')==1
            s=s.replace('KEEP(*(.ram_text*))','KEEP(*(.ram_text*)) *(.fetch_test)',1)
        else:
            assert s.count('*(.text*)')==1
            s=s.replace('*(.text*)','*(.text*) *(.fetch_test)',1)
        assert s!=script and s.count('cold_lab_')==3 and s.count('cold_slot3_')==3 and s.count('cold_fittab_')==3
        ld.write_text(s)
        if mp.exists():mp.unlink() # Never certify a stale failed-link map.
        try:
            real(tool,*cmd,'-Map='+str(mp))
        except SystemExit:
            if mp.is_file():
                audit_meta()
                audit_fdata_map()
                audit_fittab_map()
            if mp.exists():
                m=mp.read_text()
                def loc(n):
                    return int(re.search(r'^\s*(0x[0-9a-f]+)\s+'+re.escape(n)+r' = \.',m,re.M)[1],16)
                beg=0x020a9000
                lo,hi=loc('cold_xdata_tail_start'),loc('cold_xdata_tail_end')
                assert loc('_xdata_start')==beg and loc('_xdata_end')==beg+7698
                assert lo==beg+7700 and hi==beg+8180
                for name,addr,n in (('.xdata.0hdr',beg,16),('.xdata.font',beg+16,7682)):
                    z=re.findall(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M)
                    assert len(z)==1 and tuple(int(i,16) for i in z[0])==(addr,n)
                offsets=[]
                for name,expected in xdata_tables.items():
                    z=re.findall(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M)
                    assert len(z)==1,(name,z)
                    addr,n=tuple(int(i,16) for i in z[0])
                    assert n==expected and lo<=addr<=hi-n,(name,addr,n)
                    offsets.append((addr,addr+n))
                    print('XDATA map-only',b.MODEL,name,hex(addr),n,flush=True)
                assert sorted(offsets)[0][0]==lo and sorted(offsets)[0][1]==sorted(offsets)[1][0] and sorted(offsets)[1][1]==hi
                print('XDATA map-only extent',b.MODEL,hi-beg,'remaining',8192-(hi-beg),flush=True)
            raise
        real('common/bin/objcopy','-O','binary','-j','.lab_morph',elf,out/'lab_morph.bin')
        payload=(out/'lab_morph.bin').read_bytes()
        assert 18472 < len(payload) <= 0x5000
        print('quarantine morph section',b.MODEL,len(payload),hashlib.sha256(payload).hexdigest(),flush=True)
        for n in syms: real('common/bin/objcopy','-O','binary','-j','.'+n,elf,out/(n+'.bin'))
        return [(out/(n+'.bin')).read_bytes() for n in syms]
    a=link(bytes(32),bytes(32),bytes(32),0)
    mt=mp.read_text()
    def sym(n):
        return int(re.search(r'^\s*(0x[0-9a-f]+)\s+'+re.escape(n)+r' = \.',mt,re.M)[1],16)
    lo,hi=(sym(n)-0x020A9000 for n in ('cold_xdata_tail_start','cold_xdata_tail_end'))
    assert sym('_xdata_end')-0x020A9000==7698
    assert lo==7700 and hi==8180 and len(a[5])==hi
    for n,size in xdata_tables.items():
        addr,count=(int(x,16) for x in re.search(r'^ '+re.escape(n)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',mt,re.M).groups())
        assert count==size and 0x020A9000+lo<=addr<=0x020A9000+hi-count,(n,addr,count)
    xcrc=zlib.crc32(a[5][lo:hi])
    p,p2,p3=(a[-2][32:],a[-1][32:],a[4][32:])
    assert cold909_end+sum(relocated.values())<=len(a[-2])<=61440
    assert 32+sum(pair.values())<=len(a[-1])<=32768
    assert 32+sum(v[0] for v in exact_fittab.values())<=len(a[4])<=4096
    assert a[-2][:32]==a[-1][:32]==a[4][:32]==bytes(32)
    i={'elite':1,'pro':2}[b.MODEL]
    ident=hashlib.sha256(b'D9-READONLY-LAB-3'+bytes([i])+b''.join(struct.pack('<I',len(x))+x for x in (*a[:4],a[5],p,p2,p3))).digest()[:12]
    h=struct.pack('<4s4I12s',b'D9RD',2,i,len(p),zlib.crc32(p),ident)
    h2=struct.pack('<4s4I12s',b'D9T3',2,i,len(p2),zlib.crc32(p2),ident)
    h3=struct.pack('<4s4I12s',b'D9FT',2,i,len(p3),zlib.crc32(p3),ident)
    z=link(h,h2,h3,xcrc)
    n=z[0].index(h)
    assert z[0].count(h)==z[0].count(h2)==1 and z[0].index(h2)==n+32
    assert z[0].count(h3)==1 and z[0].index(h3)==n+64
    assert z[0][:n]+bytes(100)+z[0][n+100:]==a[0] and z[1:4]==a[1:4] and z[5]==a[5]
    assert z[0][n+96:n+100]==struct.pack('<I',xcrc)
    assert z[4]==h3+p3
    for name,size in xdata_tables.items():
        entries=re.findall(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',mt,re.M)
        assert len(entries)==1
        address,count=map(lambda v:int(v,16),entries[0])
        k=address-0x020a9000
        assert count==size and z[5][k:k+size]==a[5][k:k+size]
        print('XDATA linked-sha256',b.MODEL,name,size,hashlib.sha256(z[5][k:k+size]).hexdigest(),flush=True)
    assert z[-2]==h+p and z[-1]==h2+p2
    audit_meta(z[-1],a[-1])
    m=mp.read_text()
    for name,expected in relocated.items():
        aa,sz=(int(x,16) for x in re.search(r'^ '+re.escape(name)+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',m,re.M).groups())
        assert sz==expected and 0x020E1000+cold909_end<=aa and aa+sz<=0x020E1000+len(z[-2]),(name,aa,sz)
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
        assert 32<=off<=cold909_end-size and (size,hashlib.sha256(data).hexdigest())==streams[name]
        assert len(data)==size and z[-2][off:off+size]==data
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
b.CFLAGS += ['-DX0X_COLD_COMPANION_LAB=1','-DX0X_COLD_SLOT3_LAB=1','-DX0X_COLD_FITTAB_LAB=1','-DX0X_909_HUFF_DIAG=1']
b.CFLAGS.extend(["-DX0X_S1_FEEDBACK_PROBE=1","-DX0X_S1_DELAY_TRACK_TEST=1","-DX0X_SJ6_FILTER_VEL_PROBE=1","-DX0X_SJ6_VELOCITY_PORT=1","-DX0X_SJ6_NATIVE_RANDOM2=1"])
b.CFLAGS.append("-DX0X_SJ6_NATIVE_POSITIVE_GATE=1")
b.CFLAGS += ["-DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1","-DX0X_S1_MOD_RATE_PROBE=1","-DX0X_S1_BEND_RANGE_PROBE=1","-DX0X_S1_AUTOPAN_PROBE=1","-DX0X_S1_AUTOPAN_PORT=1","-DX0X_SJ6_MONO32_PROBE=1"]
b.CFLAGS.extend(["-DX0X_S1_FEEDBACK_PROBE=1","-DX0X_S1_DELAY_TRACK_TEST=1","-DX0X_SJ6_FILTER_VEL_PROBE=1","-DX0X_SJ6_VELOCITY_PORT=1","-DX0X_SJ6_NATIVE_RANDOM2=1"])
b.CFLAGS.append("-DX0X_SJ6_NATIVE_POSITIVE_GATE=1")
b.ORIGINAL_SYNTH_OZ_UNITS |= {'chicago_selector','chicago_arp_control','chicago_note_context','miami_geometry','miami_fx_conversion'}
b.CFLAGS.extend(Path(os.environ['DNBOX_CONTROL_FLAGS']).read_text().split())
if os.environ.get("DNBOX_CHICAGO_MOD_SPEED_COMPACT") == "1": b.CFLAGS.append("-DDNBOX_CHICAGO_MOD_SPEED_COMPACT=1")
if os.environ.get("DNBOX_CHICAGO_DRIVE_COMPACT") == "1": b.CFLAGS.append("-DDNBOX_CHICAGO_DRIVE_COMPACT=1")
if os.environ.get("DNBOX_CHICAGO_MOD_PALETTE") == "1": b.CFLAGS.append("-DDNBOX_CHICAGO_MOD_PALETTE=1")
if os.environ.get("DNBOX_MIAMI_STOCK_CAP_COMPACT") == "1": b.CFLAGS.append("-DDNBOX_MIAMI_STOCK_CAP_COMPACT=1")
if os.environ.get("DNBOX_MIAMI_DECIMATOR_COMPACT") == "1": b.CFLAGS.append("-DDNBOX_MIAMI_DECIMATOR_COMPACT=1")
b.CFLAGS.append('-DX0X_CHICAGO_FAST32=1')
if os.environ.get('DNBOX_TANH_SPLIT_LAB') == '1':
    b.CFLAGS.append('-DX0X_TANH_SPLIT_LAB=1')
b.CFLAGS += ["-DX0X_COLD_XDATA_TAIL_LAB=1"]
b.CFLAGS.append('-DDNBOX_SHARED_LE32_LAB=1')
b.generate=lambda:None
b.build_loader=lambda:None
b.tc=seal
try:b.main()
except Stop:pass
