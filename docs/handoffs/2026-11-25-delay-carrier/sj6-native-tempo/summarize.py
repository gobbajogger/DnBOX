"""Check native clock readback and time-marked audible echo against a bypass control."""
import json,subprocess,sys
from pathlib import Path
import numpy as np
h=Path(sys.argv[1]); root=h.parent.parent
reads=lambda p:json.loads(p.read_text())
phases=['early','wet','dry']
models={}
for model in ('elite','pro'):
    shots=Path((h/f'{model}.output').read_text().strip())
    elf=root/'rehydrated-reverb/delay-center-exact-wet/tree/labout'/model/'never-install.elf'
    data={}
    pcm={}
    for phase in phases:
        f=shots/phase
        prefix=h/f'{model}-{phase}'
        ram=json.loads(subprocess.check_output([sys.executable,str(h/'ram-state.py'),str(elf),str(f/'ram.bin'),str(h.parent/'native/delay-layout.s'),phase]))
        ctl=reads(prefix.with_suffix('.controls.json'))
        timing=reads(prefix.with_suffix('.timing.json'))
        log=prefix.with_suffix('.emulator.log').read_text()
        assert 'skipped 0 cycles' in log and 'flash: 0 erases / programs' in log
        assert timing['raw_late']==timing['xruns']==0
        data[phase]=dict(control=ctl,ring=ram,timing=timing)
        pcm[phase]=np.fromfile(f/'audio.s16le',dtype='<i2').reshape(-1,2).astype('i4')
    assert pcm['wet'].shape==pcm['dry'].shape
    # 10-ms peak windows, after boot-only sound. A single 40-ms MIDI burst
    # is the only attack; later peaks are absent in the bypass control.
    peaks=lambda arr: np.max(np.abs(arr[:len(arr)//441*441].reshape(-1,441,2)),axis=(1,2))
    wet,dry=peaks(pcm['wet']),peaks(pcm['dry'])
    onset=next(i for i in range(170,207) if dry[i]>24)
    idx=[]
    for lag in (25,49,73):
        region=range(onset+lag-1,onset+lag+3)
        k=max(region,key=lambda i:wet[i])
        assert wet[k]>max(20,dry[k]*8)
        idx.append(dict(peak_bin_from_dry_onset=k-onset,wet_peak=int(wet[k]),dry_peak=int(dry[k])))
    assert len({v['control']['clock'] for v in data.values()})==1
    assert data['early']['control']['owner']==data['wet']['control']['owner']==data['dry']['control']['owner']
    models[model]={'shots':data,'pcm_10ms_windows':idx,'one_dry_onset_bin':onset}
print(json.dumps(models,indent=2))
