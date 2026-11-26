import hashlib, json, sys
from pathlib import Path
import numpy as np
here=Path(__file__).parent
probe=json.loads((here/'scored.json').read_text())
source=Path(sys.argv[1])
modes={'f250':.25,'f500':.5,'f1000':1.0}
out={}
for key,f in modes.items():
    case=source/f'native-jupiter-sine-gain-authored-compressor-{key}-off'
    a=json.loads((case/'capture.json').read_text())
    raw=np.load(case/'output.npy',allow_pickle=False)
    assert hashlib.sha256(raw.tobytes()).hexdigest()==a['pcm_sha256']
    assert a['component_version']==67072 and a['rate']==44100 and a['osc2_fader']==f
    assert all(a['forced_controls'][str(i)]==0 for i in [11,16,17,22,23,24,40,41,107,108,109])
    master=a['actual_master']
    peak=float(abs(np.fft.rfft(raw[0,24576:46626].astype(np.float64))[220]/11025))
    input_peak=probe[key]['peak_phase_estimate']
    out[key]={'output_h1_peak':peak,'crusher_equivalent_h1_peak':input_peak,'apparent_dry_transfer':peak/input_peak,
              'public_master_raw':master,'difference_from_raw_master_dB':float(20*np.log10(peak/input_peak/master))}
print(json.dumps(out,indent=2))
