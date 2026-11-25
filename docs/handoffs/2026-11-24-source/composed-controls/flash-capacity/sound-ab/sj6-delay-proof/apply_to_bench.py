"""Apply the standalone delay overlay to a private copy of the owner ARM64 bench."""
import argparse
from pathlib import Path
import shutil

def apply(bench):
    bench=Path(bench)
    fx=bench/'fx.py'; swift=bench/'swiftrt.py'
    s=fx.read_text(); t=swift.read_text()
    if 'from sj6_stereo_delay import delay' not in s:
        start=s.index('def delay(x, o, enabled):')
        end=s.index('def fdn_reverb',start)
        s=s[:start]+'from sj6_stereo_delay import delay\n\n\n'+s[end:]
        s=s.replace('None of this is SuperJ6 code: each\nstage is a simple textbook approximation of the AudioKit / SuperJ6 node it stands in for (see README).','SuperJ6 delay uses the Build9-checked host core. Most other stages still approximate their original node.')
    if "'setPingPong:'" not in t:
        before="'setMonophonic:', 'setLegato:'"
        if before not in t: raise ValueError('unknown Swift runtime version')
        t=t.replace(before,before+", 'setPingPong:'",1)
    for suffix in ('.py','.c'):
        name='sj6_stereo_delay'+suffix
        shutil.copyfile(Path(__file__).with_name(name),bench/name)
    fx.write_text(s); swift.write_text(t)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('bench_directory');apply(p.parse_args().bench_directory)
