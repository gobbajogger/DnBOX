from pathlib import Path
import sys,struct,json
D=Path(__file__).resolve().parent;R=D.parent.parent
sys.path.insert(0,str(R/'restored-harness-inspect/helpers'))
import guarded_replay as g
model,pin,elf,shot=sys.argv[1:];shot=Path(shot)
expect={
'elite':'1570eabfbfc984964d92ce7e0f9f46c16d834b84d2c06440cbf9cb7d5503be57',
'pro':'1533738e0fd61e27cf1ba59bc5b24cd7d7122b42136139b7f896b1194be15b67'
}
e,s,sec=g.symbols_and_sections(Path(elf));assert g.sha(e)==pin==expect[model]
r=g.regular(shot/'ram.bin')
for name in ('.ram_text','.c1_text'):
 a,n,b=sec[name];i=a-g.ELF_RAM;assert n==len(b) and r[i:i+n]==b
assert s['lab_miss_busy']==0x1c3ab9c
def words(name,n):
 i=s[name]-g.ELF_RAM;assert 0<=i<=len(r)-4*n
 return struct.unpack_from('<'+str(n)+'I',r,i)
dbg=words('x0x_dbg',13)
result=json.loads((shot/'result.json').read_text())
assert dbg[0]==0x44424731 and dbg[10]==1 and 0<dbg[5]==words('audio_xruns',1)[0]
assert result['raw_late']==result['xruns']==dbg[5] and result['skipped_cycles']==0 and result['peak']>0
assert result['core1']['enabled']==1 and result['core1']['alive']==1 and result['core1']['bad_mailbox']==0 and result['core1']['gave_up']==0
owners=json.loads((shot/'owner.json').read_text())['owners']
valid=[o for o in owners if (o.get('bank'),o.get('status'),o.get('bankB'),o.get('statusB'),o.get('arp'))==(2,1,0,1,1)
       and isinstance(o.get('cutoff'),int) and 0<=o['cutoff']<=127]
assert valid,'no genuine matching owner status; stop without additional replay'
assert len(valid)==len(owners)
a,at,b,bt,c0,c1=words('lab_miss_busy',6)
assert 0<at<=bt<=dbg[2] and (a>>16)<=(b>>16)<dbg[1]
assert g.sha(g.regular(shot/'scene.keys'))==g.SCENE
names=['909_active_completed','808_active_kit_branch','BREAK_non_silent',
 'A_Chicago_active','A_Miami_active','A_SynthOne_active','A_Juno_or_SuperJ6_active',
 'B_Chicago_active','B_Miami_active','B_SynthOne_active','B_Juno_or_SuperJ6_active',
 'C1_recorded_job','C1_808_prefix_called','bass_runtime_reject','embedded_legacy_hit','local_takeover']
def item(w,us):
 i=w>>16;m=w&0xffff
 return dict(half_ordinal=i,renderer_work_frame_interval=[i*512,(i+1)*512],
    elapsed_us=us,mask=hex(m),applied_busy_flags=[n for j,n in enumerate(names) if m>>j&1])
nor=g.regular(shot/'disposable.bin')
ps={}
for i in (0,1,2,3,5,6):
 q=0x9d000+8192*i; sz=g.u32(nor,q+12)
 ps[str(i)]=g.sha(nor[q:q+32]+nor[q+256:q+256+sz])
print(json.dumps(dict(model=model,elf=pin,ram=g.sha(r),unchanged_disposable_nor=g.sha(nor),
 scene=g.SCENE,project_input_records=ps,first_actual_miss=item(a,at),worst_actual_miss=item(b,bt),
 aggregate=result,valid_owner_points=len(valid),valid_owner_before_external_tape=sum(o['ms']<2220 for o in valid),
 first_valid_owner_reply_ms=min(o['ms'] for o in valid),legacy_timeline_words=words('d9_test_timeline',4),
 caveat='Instrumented only; intervals counted in completed renderer work, not absolute DAC/producer input alignment. Job flags are completed-work lower bound; no matched causal A/B, no release. No claim of live SuperJ6 when not observed.'),indent=2))
