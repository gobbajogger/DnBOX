"""Read-only archive, not a gate pass and not per-half attribution."""
import json,struct,sys
from pathlib import Path
root=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(root/'restored-harness-inspect/helpers'))
import guarded_replay as g
for m,sha in map(str.split,(root/'ble-current-pair-check/pins.txt').read_text().splitlines()):
    elf=root/'ui-native-table-share/tree/widen-native'/m/'never-install.elf'
    blob,sym,sec=g.symbols_and_sections(elf); assert g.sha(blob)==sha
    trace=(root/f's1-native-909-bitwindow/base/{m}-replay.log').read_text().splitlines()
    d=Path(next(s for s in trace if s.startswith('/tmp/')))
    assert g.sha(g.regular(d/'scene.keys'))==g.SCENE
    ram=g.regular(d/'ram.bin')
    for name in ('ram_text','c1_text'):
        a,n,b=sec['.'+name];i=a-g.ELF_RAM; assert len(b)==n and ram[i:i+n]==b
    names='magic halves max_us nested in_audio late timer_irqs ui_frames last_us cpu_q8 boots stage crash_seen'.split()
    off=sym['x0x_dbg']-g.ELF_RAM
    v=dict(zip(names,struct.unpack_from('<13I',ram,off)))
    metrics=json.loads((d/'result.json').read_text())
    assert v['max_us']==metrics['worst_us'] and v['late']==metrics['raw_late']==metrics['xruns']
    absent=['x0x_worst','dbg_tmr','eng_dbg_nsub']
    assert all(k not in sym for k in absent)
    t=json.loads((root/f's1-native-stage-interval/{m}-intervals.json').read_text())
    # Measured whole-block upper-bound removal of distinguishable non-overlap finish,
    # without interpreting IRQ as exclusive or summing interval totals.
    serial=dict(core0_909_us=t['job_exec_us'][0][0],core0_chicago_us=t['job_exec_us'][0][3],core1_s1_us=t['job_exec_us'][1][2],
      already_c1_808_us=t['already_split_808_body_interval_us'],ordered_wait=t['ordered_wait_us'], optimistic_remaining=t['optimistic_total_including_already_split_808_us'])
    print(json.dumps(dict(model=m,elf=sha,ram_sha=g.sha(ram),scene=g.SCENE,aggregate=v,
     late_timestamps_available=False,absent=absent,available_measured_intervals=dict(elf=t['elf_sha256'],worst=t['worst_us'],span_count=len(t['spans']),plan=t['masks'],insight=serial))))
