"""Re-link a disposable external-asset lab image; never package or install."""
import hashlib,os,re,struct,zlib
from pathlib import Path
from tools import build as b
old=('compact_chicago_factory_codes','original_chicago_factory_names',
     'original_program_value_words','DV_XY_PACKED','DV_POLY_PACKED','compact_miami_name_offsets',
     'compact_miami_factory_codes','compact_miami_names','smc_arc_pixels',
     'UI_PICTOGRAM_LOW','UI_PICTOGRAM_HIGH','CK_SCALE_N','CK_CHORD_N','CK_CD_NAMES','CK_RATE_N',
     'CK_RHY_N','CK_FLD_N','CK_ORD_N','original_chicago_factory_name_offset','CK_CHORD_T','CK_CD_TONES',
     'APC40_LCD565','APC40_PAD_OWNER','DV_BODY','DV_VOICE_NAMES','CK_MODE_N',
     'ui_original_juno_titles','ui_original_chicago_titles','ui_original_miami_titles','ui_original_synthone_titles')
# Pure UI descriptors, render geometry and names. Exclude PALETTES, fonts, loader,
# shared string pools, boot/OTA/USB, time/sample tables, and active DSP math.
extra=('sj6_knob_desc.knobs','ARP_P','GEN_P','BRKSLICE_P','ACT_P','SONG_P','MIDICH_P','PARTON_P','SEQ_P',
       'RATE_N','THEME_N','BATT_N','BTSTAT_N','X0X_NOTE_N','ARP_SPEED_N','ARP_STEPS_N','ARP_INTERVAL_N',
       'semantic_amount_glyph.ratios','all_glyph_semantics.gap','GLOBAL_SECTIONS',
       'pref_name.BRK_ID','pref_name.BRK_SHOW','pref_name.SEQN','pref_name.PATN','pref_name.MIXN',
       'ui_original_name.juno','ui_original_name.chicago','ui_original_name.miami','CK_LIST','dv_bank_slots',
       'BTN_LABEL','SMCFX_BUS_NAME','SMCFX_LONG','SMCFX_SHORT','VIEW_N',
       'TB3PO_SCALE_NAMES','MIX_P','fx_div_names','RATIO_N','drive_amount_glyph.wave',
       'APC40_LCD_IDLE565','smc_mmc_note.tail','ui_original_chicago_rows','ui_original_miami_rows',
       'ui_original_synthone_rows','ui_original_juno_rows','X0X_NOTE_PACK','SMCFX_PAGE','SMCFX_SUIT',
       'division_pad_label.labels','division_shape_label.labels','global_section.rows')
assert not set(old+extra) & {'ui_material','PALETTES','B_NAME','E_NAME','PANEL_DEFAULT','FONT_T_DATA'}
real=b.tc
class Stop(Exception):pass
def go(tool,*args):
    if tool!='pi32v2/bin/ld':return real(tool,*args)
    src=Path(b.OUT)/'candidate.map'
    m=src.read_text()
    hits={}
    for n in old+extra:
        q=re.search(r'^ \.rodata\.'+re.escape(n)+r'\s+0x0*20[a-f0-9]+\s+0x([a-f0-9]+)',m,re.M)
        if q:hits[n]=int(q[1],16)
    print('retained selected static payload',sum(hits.values()),sorted(set(old+extra)-hits.keys()),flush=True)
    cmd=list(args); i=cmd.index('-T')+1
    script=Path(cmd[i]).read_text()
    out=Path(os.environ['FIT_LAB_OUTPUT'])/b.MODEL;out.mkdir(parents=True,exist_ok=True)
    elf=out/'never-install.elf'
    ld=out/'lab.ld';cmd[i]=ld;cmd[-1]=elf
    # Pure UI tables from the existing guarded allowed set, with no new owners.
    selected=('DV_XY_PACKED','DV_POLY_PACKED','smc_arc_pixels','UI_PICTOGRAM_LOW',
              'UI_PICTOGRAM_HIGH','CK_CD_NAMES','APC40_PAD_OWNER','ACT_P','smc_mmc_note.tail')
    assert set(selected) <= hits.keys()
    tables=''.join(' *(.rodata.'+k+')' for k in hits if k not in selected)+' *(.lab_ui_copy.*)'
    # Only pure menu descriptor constants emitted by describe_pages and its
    # palette-independent normal draw corner data; existing LAB bounds checked.
    additional=tuple(re.findall(r'^ \.rodata\.(describe_pages\.refs(?:\.[0-9]+)?)\s+0x0*20[0-9a-f]+\s+0x[0-9a-f]+',m,re.M))
    assert additional and len(additional)==len(set(additional))
    tables+=' *(.rodata.CORNER_PACKED)'+''.join(' *(.rodata.'+k+')' for k in additional)
    tail=''.join(' *(.rodata.'+k+')' for k in selected)+' *(.lab_help)'
    def link(head0,head1):
        words=lambda head:' '.join('LONG(0x%08x);'%w for w in struct.unpack('<8I',head))
        a,t=words(head0),words(head1)
        meta=(' . = ALIGN(4); cold_lab_expected = .; '+a+
              ' . = ALIGN(4); cold_lab_tail_expected = .; '+t+' ')
        # Use unchanged existing address arithmetic only in this fixture. Slot
        # window bounded to exact prior used footprint; no extra allocation.
        s=script.replace('    XIP    (rx)',
            '    LABCOLD (r) : ORIGIN = 0x020AF000, LENGTH = 13698\n'
            '    LABFIT (r) : ORIGIN = 0x020FA000, LENGTH = 0x1000\n    XIP    (rx)')
        assert s.count(' . = ALIGN(4); _etext')==1
        s=s.replace(' . = ALIGN(4); _etext',meta+'. = ALIGN(4); _etext')
        assert s.count('SECTIONS\n{\n')==1
        s=s.replace('SECTIONS\n{\n','SECTIONS\n{\n'+
              ' .lab_cold : { cold_lab_start = .; '+a+tables+' cold_lab_end = .; } > LABCOLD\n'+
              ' .lab_fit : { cold_lab_tail_start = .; '+t+tail+' cold_lab_tail_end = .; } > LABFIT\n')
        ld.write_text(s)
        real(tool,*cmd,'-Map='+str(out/'lab.map'))
        names=('text','lab_cold','lab_fit','ram_text','c1_text','data','xdata')
        for name in names:
            real('common/bin/objcopy','-O','binary','-j','.'+name,elf,out/(name+'.bin'))
        return tuple((out/(k+'.bin')).read_bytes() for k in names)
    Z=bytes(32)
    app,a,f,*rest=link(Z,Z)
    parts=(a[32:],f[32:])
    assert 32<=len(a)<=13698 and 32<=len(f)<=4096
    model={'elite':1,'pro':2}[b.MODEL]
    sections=(app,*rest,*parts)
    ident=hashlib.sha256(b'X0X-LAB-FIXTURE-SPLIT-V2'+bytes([model])+
        b''.join(struct.pack('<I',len(x))+x for x in sections)).digest()[:12]
    def header(magic,p):
        return struct.pack('<5I12s',magic,2,model,len(p),zlib.crc32(p),ident)
    heads=(header(0x444c4f43,parts[0]),header(0x32425453,parts[1])) # COLD, STB2 not FTB1
    app2,a2,f2,*rest2=link(*heads)
    assert rest2==rest and len(app2)==len(app)
    assert a2==heads[0]+parts[0] and f2==heads[1]+parts[1]
    for h in heads:
        assert app2.count(h)==1
        app2=app2.replace(h,Z,1)
    assert app2==app
    sealed_map=(out/'lab.map').read_text()
    for internal in ('ui_material','PALETTES','B_NAME','E_NAME','FONT_T_DATA'):
        sym=re.search(r'^ \.rodata\.'+internal+r'\s+0x([a-f0-9]+)',sealed_map,re.M)
        assert sym and 0x02000120 <= int(sym[1],16) < 0x02087000,(internal,'missing or not in app')
    print('guarded FITTAB split sealed; live sector0/sector1, ids', len(a2), len(f2), *(h.hex() for h in heads), flush=True)
    raise Stop
if not os.environ.get('FIT_LAB_UNGUARDED_ACCOUNT'): b.CFLAGS += ['-DX0X_COLD_COMPANION_LAB=1']
b.CFLAGS += ['-DX0X_COLD_HELP_LAB=1', '-DX0X_COLD_COMPANION_SPLIT_LAB=1', '-DX0X_COLD_UI_LIT_LAB=1']
b.generate=lambda:None
b.build_loader=lambda:None
b.tc=go
try:b.main()
except Stop:pass
