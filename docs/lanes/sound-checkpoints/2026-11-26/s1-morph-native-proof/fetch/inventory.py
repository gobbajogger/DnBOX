"""Read only flash/RAM capacity ledger for the guarded existing Synth One release candidates."""
import hashlib,json,math,re,subprocess
from decimal import Decimal,ROUND_CEILING
from pathlib import Path
r=Path(__file__).resolve().parent.parent
ref=r/'s1-morph-native-sched'
w=r/'s1-morph-placement'/'result.json'
probe=json.loads(w.read_text())
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
rows=[]
for model in ('elite','pro'):
    out=ref/'tree/widen-native'/model
    log=(ref/(model+'.log')).read_text()
    pin=re.search(r'guarded \w+ XIP (\d+) free (\d+) pool usable (\d+).* ELF ([a-f0-9]{64})',log)
    size,slack,pool_map=map(int,pin.groups()[:3]);elf=out/'never-install.elf'
    assert digest(elf)==pin[4] and size+slack==552672
    symbols={}
    for line in subprocess.check_output(['readelf','-sW',elf],text=True).splitlines():
        q=line.split()
        if len(q)>=8 and q[6]!='UND' and q[2].isdigit():
            symbols[q[7]]=(int(q[1],16),int(q[2]))
    assert symbols['_pool_end'][0]-(0x01c20000)+(pool_map+8192)==0x54000
    assert symbols['_xpool_end'][0]-symbols['_xpool_start'][0]==9216
    rows.append(dict(model=model,elf=digest(elf),app_used=size,app_free=slack,pool_total_free=pool_map+8192,
      protected_pool_reserve=8192,usable_extra_pool=pool_map,ramtext_tail=0x01c01800-symbols['_c1_end'][0],
      xpool_used=9216,ordinary_data_bss_ram_unallocated=0x01c20000-symbols['_bss_end'][0],existing_companions={k:len((out/(k+'.bin')).read_bytes()) for k in ('lab_fdata','lab_slot3','lab_fittab','xdata','lab_morph')}))
for q in rows:
    assert q['existing_companions']==dict(lab_fdata=61432,lab_slot3=32764,lab_fittab=4072,xdata=8180,lab_morph=20372)
cap={k:{'used':n,'capacity':t,'spare':t-n} for (k,n,t) in [('FDATA',61432,61440),('diagnostic SLOT3',32764,32768),
 ('FITTAB',4072,4096),('XDATA',8180,8192)]}
examples=[]
for t in (Decimal('0.2162'),Decimal('2.4')):
    n=sum(int((44100*t*d).to_integral_value(rounding=ROUND_CEILING)) for d in (2,2,1,1))
    h=4*n
    examples.append(dict(seconds=float(t),four_float32_delay_line_history_lower_bound=h,
       with_one_float32_revsc_history=h+98760,with_two_independent_owners=2*(h+98760),
       with_one_float32_revsc_and_widen=h+98760+8820,with_two_owners_plus_widen=2*(h+98760+8820)))
d=dict(models=rows,existing_companion_capacity=cap,retained_diagnostic_unique_spare_sum=sum(x['spare'] for x in cap.values()),
 current_no_audited_production_immutable_region=True,
 tail_diagnostic=dict(start_hex='0xD1000',end_hex='0xD6000',capacity=20480,
   raw_codec_plus_table=20372,proposed_additional_manifest=32,spare_if_versioned=76,
   original_slot1_upload_capacity=81920,slot1_capacity_after_repartition=61440,
   stock_five_breaks_remain_in_offline_test=True,unsafe_under_current_writable_range=True),
 native_compact_fx_borrowed_arena_per_owner=4252,native_half_precision_widen_built_into_its_backend_per_owner=4410,
 two_owner_hypothetical_available_if_compact_replaced=2*4252+min(q['usable_extra_pool'] for q in rows),
 original_vdelay_allocated_history=4*(44100*10+1)*4,original_revsc_one_owner_history=98760,
 original_widen_one_50ms_float32_history=4*44100//20,
 lower_bounds_before_alignment_state=examples,
 scope='Runtime delay/reverb histories are mutable each sample; read-only storage cannot satisfy these bounds. No flash/PSRAM writes or capacity shrinking.')
print(json.dumps(d,indent=2))
