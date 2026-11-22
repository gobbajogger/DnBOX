"""Replay both original-controlling frontends and summarize their post-restore reverb values."""
from collections import Counter
from pathlib import Path
import json
import subprocess
import sys


def measure(repo, build, out):
    # Reuse the production qualification's complete original 144-write replay.
    source=(repo/'experiments/sound-hardware-v1/synthone-delay/targets.cpp').read_text()
    a=source.index('    std::puts("index,on,feedback,seconds,');b=source.index('    for (unsigned row',a)
    source=source[:a]+source[b:]
    a=source.index('        std::printf("%u",row);');b=source.index('        std::puts("");',a)
    ids=[37,38,39,40,51,123,126,129,132,135,124,127,130,133,136]
    source=source[:a]+'''        for(auto id : {'''+','.join(map(str,ids))+'''}) {
            CHECK(source.target(id)==original.target(id));
            std::printf("%a ",double(original.target(id)));
        }
'''+source[b:]
    (out/'targets.cpp').write_text(source)
    cmds=json.loads((build/'factory-build-commands.json').read_text());cmd=next(c.copy() for c in cmds if any(a.endswith('/preset_audio.cpp') for a in c))
    for flag, path in [('-c','targets.cpp'),('-o','targets.o'),('-MF','targets.d')]: cmd[cmd.index(flag)+1]=str(out/path)
    subprocess.run(cmd,check=True);cmd=cmds[-1].copy();cmd[cmd.index(str(build/'factory-6.o'))]=str(out/'targets.o');cmd[cmd.index('-o')+1]=str(out/'targets');subprocess.run(cmd,check=True)
    inputs=build/'factory-inputs'; raw=subprocess.check_output([out/'targets',inputs/'public-morphs.f32',inputs/'compact.bin',build/'commands/preset-commands.bin'],text=True)
    rows=[list(map(float.fromhex,row.split())) for row in raw.splitlines()]; assert len(rows)==694 and all(len(row)==len(ids) for row in rows)
    wet=[r for r in rows if r[0]*r[3]>0]; fmt=lambda counts:{str(k):v for k,v in sorted(counts.items())}
    return {'ids':ids,'programs':len(rows),'on_positive_mix':len(wet),'off':sum(r[0]==0 for r in rows),'on_zero_mix':sum(r[0]!=0 and r[3]==0 for r in rows),'on_full_mix':sum(r[0]*r[3]==1 for r in rows),'wet_lfo_route_counts':fmt(Counter(int(r[4]) for r in wet)),'input_dynamics_distinct_tuples':len({tuple(r[5:10]) for r in rows}),'wet_dynamics_distinct_tuples':len({tuple(r[10:15]) for r in rows}),'input_dynamics_distinct_tuples_on_wet':len({tuple(r[5:10]) for r in wet}),'wet_dynamics_distinct_tuples_on_wet':len({tuple(r[10:15]) for r in wet}),'wet_feedback_minmax':[min(r[1] for r in wet),max(r[1] for r in wet)],'wet_highpass_minmax':[min(r[2] for r in wet),max(r[2] for r in wet)]}

if __name__=='__main__':
    repo,build,out=(Path(a).resolve() for a in sys.argv[1:]);(out/'targets.json').write_text(json.dumps(measure(repo,build,out),indent=2)+'\n')
