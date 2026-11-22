"""Compare both active delay histories and their tails at the legal control extremes."""
import argparse, json, subprocess, sys
from pathlib import Path

import numpy as np

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
sys.path.insert(0,str(HERE.parent/'sj6-factory'))
from compare import channel_score

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('output',type=Path)
out=parser.parse_args().output.resolve()
out.mkdir(exist_ok=False,parents=True)
dsp=ROOT/'firmware/src/dsp'
exe=out/'duo'
subprocess.run(['cc','-O2','-ffp-contract=off','-I'+str(dsp),'-I'+str(ROOT/'build/gen'),HERE/'dual_delay.c',dsp/'fxbus.c',dsp/'x0x_param.c',dsp/'drum909.c','-lm','-o',exe],check=True)
rows=[]
for mode in range(16):
    temp=out/'pair.f32'
    subprocess.run([exe,str(mode),temp],check=True)
    data=np.fromfile(temp,'<f4').reshape(-1,4).astype('d')
    temp.unlink()
    assert data.shape==(264600,4)
    for side in (0,1):
        for name,span in [('whole',slice(None)),('tail',slice(88200,None))]:
            original,packed=data[span,side],data[span,side+2]
            rms,spec=channel_score(original,packed,44100,[20,16000])
            rows.append(dict(mode=mode,side=side,interval=name,level_db=rms,spectral_rms_db=spec,passed=bool(abs(rms)<=1 and spec<=3)))
j=dict(rate=44100,frames_per_case=264600,shared_reference='Full 16-bit DnBOX delay, including DnBOX TAPE and PING',private_reference='Identical isolated 808 private instance',cases=rows,passed=sum(x['passed'] for x in rows))
(out/'results.json').write_text(json.dumps(j,indent=2)+'\n')
print(j['passed'],len(rows))
raise SystemExit(j['passed']!=len(rows))

