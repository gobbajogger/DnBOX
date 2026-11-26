"""Only reports/fixes expected pins for the two current, successfully sealed diagnostic ELFs."""
import hashlib,json,re,subprocess,sys
from pathlib import Path
d=Path(__file__).resolve().parent
sys.path.insert(0,str(d.parent/'restored-harness-inspect/helpers'))
from guarded_replay import symbols_and_sections,sha
modes=['flash','ram'];boards=['elite','pro']
pins={m:{} for m in modes};info={}
for board in boards:
 a=[]
 for mode in modes:
    p=d/'tree'/('xip-'+mode)/board
    log=(d/(board+'-'+mode+'.log')).read_text()
    match=re.search(r'^guarded .* ELF ([a-f0-9]{64})$',log,re.M)
    if not match:raise ValueError('matching guarded link required')
    f=p/'never-install.elf';b,sy,sec=symbols_and_sections(f)
    assert sha(b)==match[1]
    pins[mode][board]=sha(b)
    ss=next(l.split() for l in subprocess.check_output(['readelf','-sW',f],text=True).splitlines() if re.search(r'\bd9_render_smp$',l))
    v,z=int(ss[1],16),int(ss[2])
    name='.text' if mode=='flash' else '.ram_text'
    assert sy['d9_render_smp']==v and z>0
    start,n,code=sec[name];o=v-start
    assert 0<=o<=n-z
    a.append(dict(mode=mode,elf_sha256=sha(b),address=hex(v),function_bytes=z,
      raw_function_sha256=sha(code[o:o+z]),code=code[o:o+z],regions={k:sha(sec[k][2][32:]) for k in ('.lab_fdata','.lab_slot3','.lab_fittab')},
      font=sha(sec['.xdata'][2][16:16+int.from_bytes(sec['.xdata'][2][4:8],'little')]),xdata_tail=sha(sec['.xdata'][2][16+int.from_bytes(sec['.xdata'][2][4:8],'little'):]),morph=sha(sec['.lab_morph'][2]),ram_text_size=sec['.ram_text'][1],
      c1_text_size=sec['.c1_text'][1],ramtext_spare=0x01c01800-sy['_c1_end']))
 assert a[0]['function_bytes']==a[1]['function_bytes']
 assert a[0]['regions']['.lab_fdata']==a[1]['regions']['.lab_fdata']
 assert a[0]['font']==a[1]['font'] and a[0]['morph']==a[1]['morph']
 code=[x.pop('code') for x in a]
 changed=[i for i,(x,y) in enumerate(zip(*code)) if x!=y]
 o=d/'tree/build'/('smk37-elite' if board=='elite' else 'smk37')/'o2/dsp_drum909.o'
 reloc=subprocess.check_output(['readelf','-rW',o],text=True)
 section=reloc.split("Relocation section '.rela.fetch_test'")[1].split('Relocation section',1)[0]
 relocations={int(i,16):name for i,name in re.findall(r'^([a-f0-9]+).* unrecognized: 5\s+\S+\s+(\S+) \+ 0$',section,re.M)}
 assert len(relocations)==9
 assert set(changed)<=set(j for i in relocations for j in (i+2,i+3))
 info[board]=dict(arms=a,changed_raw_function_byte_offsets=changed,only_data_pointer_relocations_differ=True,type5_data_pointer_sites=relocations)
print(json.dumps(info,indent=2))
(d/'pins.json').write_text(json.dumps(pins,indent=2)+'\n')
