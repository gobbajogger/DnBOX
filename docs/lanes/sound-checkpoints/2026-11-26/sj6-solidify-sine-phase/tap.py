import sys
from pathlib import Path
p,o=map(Path,sys.argv[1:]);s=p.read_text()
a='  prepare(bpm);'
assert s.count(a)==1
s=s.replace(a,a+r'''
  fprintf(stderr,"projection %a %a %u %u\n", part.dsp.juno.synth.core.layer[0].params.waveform,
    part.dsp.juno.synth.core.layer[1].params.waveform,part.dsp.juno.synth.osc1_level_code,part.dsp.juno.synth.osc2_level_code);
''')
o.write_text(s)
