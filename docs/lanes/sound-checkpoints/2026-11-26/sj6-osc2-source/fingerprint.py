"""No-fit signed held spectral discriminator; frequencies are measured for attribution."""
from pathlib import Path
text=Path('sj6-osc2-source/open_match.py').read_text().split('out={}')[0]
exec(compile(text,'verified held source closures','exec'))
out={}
for a,b in [(24576,37634),(20000,29000),(28500,37500)]:
 A={}
 for name,tape in [('actual_source_leaf',leaf),('source_suffix',z),('direct_auv3',x),('native_repeat',y),('dry_authored',old_pcm)]:
  j=tape[0,a:b].astype(float);n=len(j);q=np.abs(np.fft.rfft(np.hanning(n)*j,262144))
  extrema=np.nonzero((q[1:-1]>q[:-2])&(q[1:-1]>=q[2:]))[0]+1
  keep=sorted((k for k in extrema if 25<k*44100/262144<4000),key=lambda k:q[k],reverse=True)[:8]
  A[name]={'levels':[dict(hz=float(k*44100/262144),rms=float(q[k]/(n/4)/np.sqrt(2))) for k in keep]}
 # Same phase/cue, no fitting. Partials at exact nominal C1 harmonics (key36 with authenticated OSC2 physical slot1=-12).
 fs=44100;known=440*2**((36-12-69)/12)
 here=np.arange(a,b)/fs
 pe={}
 for h in range(1,9):
  help=np.hanning(b-a)*np.exp(-2j*np.pi*known*h*here)
  R=np.dot(x[0,a:b],help)
  V=np.dot(z[0,a:b],help)
  pe[h]={'terminal_dbc':float(20*np.log10(max(abs(R),1e-30)/max(abs(np.dot(x[0,a:b],np.hanning(b-a)*np.exp(-2j*np.pi*known*here))),1e-30))),
         'candidate_relative_dB':float(20*np.log10(max(abs(V),1e-30)/max(abs(R),1e-30))),
         'phase_deg':float(np.angle(V/R,deg=True))}
 out[f'{a}:{b}']={'peaks':A,'fixed_c1_partials':pe}
print(json.dumps(dict(scope='diagnostic spectral fingerprints on exact native PCM and unchanged actual source renderer; peak locations are observations only, no model calibration',windows=out),indent=2))
