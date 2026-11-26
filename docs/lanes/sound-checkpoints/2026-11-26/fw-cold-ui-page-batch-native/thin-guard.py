import json, sys, struct, importlib.util, subprocess, re
from pathlib import Path
base,new,model,mode=Path(sys.argv[1]),Path(sys.argv[2]),sys.argv[3],sys.argv[4]
R=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R/'restored-harness-inspect/helpers'))
import guarded_replay as g
h={'elite':'0ffdc7faf4f8ecace4e119accce10ecb64dac16122d2586d323c388da305bd7c',
   'pro':'ddbcae26a746c6ee2f4ccae382ceb4762d06f42d3544875862dcb75f94c3ea51'}
a,sa,A=g.symbols_and_sections(base);b,sb,B=g.symbols_and_sections(new)
assert mode in ('staging','candidate')
if mode=='staging': assert g.sha(a)==h[model]
else: assert g.sha(a) in {'aa2cacb33abf17bf35b287db214f82185f0bbc8a226f2daad63fb814ce66ac69','34537d270385218c6eddc08ccf1d752a558a69c39385db7746c3867c594beb86'}
spec=importlib.util.spec_from_file_location('read_only_pack',R/'fw/tools/fm1_rescue.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
pkg=R/'out'/f'DnBOX-0.0.27-SMK37-{model}-73749f55-test.fwsc'
for elf,raw in ((base,a),(new,b)):g.project(module,model,pkg,elf,g.sha(raw))
old_img,_=g.app_sections(sa,A);new_img,_=g.app_sections(sb,B)
assert A['.pool'][0]==B['.pool'][0]==0x01c20000
delta=B['.pool'][1]-A['.pool'][1]; free=0x54000-B['.pool'][1]
assert delta==0 and free>=8192+512,(delta,free)
assert A['.bss'][:2]==B['.bss'][:2]
assert sa['_xpool_start']==sb['_xpool_start'] and sa['_xpool_end']==sb['_xpool_end']==sb['_guard0']
assert not any(k in sb for k in ('lab_miss_busy','lab_first_cue','eng_terminal_claim','early_909_claim'))
def named_objects(elf):
    out={}
    for line in subprocess.check_output(["readelf","-sW",elf],text=True).splitlines():
        row=line.split()
        if len(row)>=8 and row[3]=="OBJECT" and row[2].isdigit() and int(row[2]):
            out.setdefault(int(row[1],16),set()).add((row[7],int(row[2])))
    return out
old_obj,new_obj=map(named_objects,(base,new))
def pointed_text(text, value):
    i=value-g.START
    assert 0<=i<len(text), hex(value)
    end=text.find(b"\0",i,i+192)
    assert end>=i, (hex(value),text[i:i+10])
    name=text[i:end]
    name.decode('utf-8')
    assert all(c>=32 and c!=127 for c in name), (hex(value),name)
    return name
def word_payload(section,u,v,depth):
    assert depth<=3,(section,hex(u),hex(v))
    ta,tb=A['.text'][2],B['.text'][2]
    i,j=u-g.START,v-g.START
    assert 0<=i<len(ta) and 0<=j<len(tb), (section,hex(u),hex(v))
    raw_a=old_obj.get(u,set());raw_b=new_obj.get(v,set())
    obj=sorted(raw_a & raw_b)
    if not obj:
        # LLVM may add/remove a numeric local disambiguator after unrelated
        # UI descriptors move. Keep exact size and recursive pointed bytes;
        # only permit numeric suffix changes for the known waveform labels.
        def family(rows):
            return {(re.sub(r'\.\d+$','',n),z) for n,z in rows
                    if re.fullmatch(r'sj6_knob_desc\.wave(?:\.\d+)?',n)}
        obj=sorted(family(raw_a)&family(raw_b))
    if obj:
        n,z=obj[0]; ax,bx=ta[i:i+z],tb[j:j+z]
        assert len(ax)==len(bx)==z
        if ax!=bx: words_equivalent(n,0,ax,bx,depth+1)
        return n+' '+str(sorted(raw_a))+' → '+str(sorted(raw_b))
    ax,bx=pointed_text(ta,u),pointed_text(tb,v)
    assert ax==bx,(section,hex(u),hex(v),ax,bx)
    return ax.decode('utf-8')
def words_equivalent(section, off, x, y, depth=0):
    assert len(x)==len(y) and len(x)%4==0,section
    shifted=[]
    for i in range(0,len(x),4):
        u,v=g.u32(x,i),g.u32(y,i)
        if u==v: continue
        why=word_payload(section,u,v,depth)
        shifted.append((hex(off+i),hex(u),hex(v),why))
    return shifted
relocs={}
for section in ('.lab_fdata','.lab_slot3','.lab_fittab'):
    assert A[section][:2]==B[section][:2] and A[section][2][0:16]==B[section][2][0:16],section
    relocs[section]=words_equivalent(section,32,A[section][2][32:],B[section][2][32:])
assert A['.lab_morph']==B['.lab_morph']
assert A['.xdata'][:2]==B['.xdata'][:2]
xt=sa['cold_xdata_tail_start']-A['.xdata'][0]; assert xt==sb['cold_xdata_tail_start']-B['.xdata'][0]
assert A['.xdata'][2][:xt]==B['.xdata'][2][:xt]
relocs['.xdata']=words_equivalent('.xdata',xt,A['.xdata'][2][xt:],B['.xdata'][2][xt:])
assert len(new_img)<=g.MAX
if mode=='staging': assert len(new_img)-len(old_img)==64
print(json.dumps(dict(mode=mode,model=model,off=g.sha(a),on=g.sha(b),app_max=g.MAX,off_app_free=g.MAX-len(old_img),
   on_app_free=g.MAX-len(new_img),raw_pool_delta=delta,ordinary_bss_delta=B['.bss'][1]-A['.bss'][1],ordinary_gap_before_pool=B['.pool'][0]-(B['.bss'][0]+B['.bss'][1]),pool_actual_free=free,
   remaining_beyond_hard_8704=free-(8192+512), xpool_unchanged=True,guarded_companions_equivalent_except_proven_text_and_named_object_pointer_relocations=True,
   relocation_count={k:len(v) for k,v in relocs.items()},
   link_only=True),indent=2))
