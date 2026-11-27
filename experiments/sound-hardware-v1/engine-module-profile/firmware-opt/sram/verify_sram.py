from pathlib import Path
import json, sys
import guarded_replay as g
out=Path(sys.argv[1]); w=Path(sys.argv[2])
for m in ('elite','pro'):
    b,s,sec=g.symbols_and_sections(out/m/'never-install.elf')
    a,n,contents=sec['.ram_text']
    o=s['voice_run_kr'];refresh=s['voice_kr_refresh']
    assert a<=o<a+n and g.START<=refresh<g.START+g.MAX
    assert s['_rt_end']<=0x01c01800
    assert sec['.pool'][1]<=0x54000-8192-512
    where=a-g.ELF_RAM
    r=(w/f'{m}-offset--1'/'ram.bin').read_bytes()
    assert r[where:where+n]==contents, (m,'boot SRAM copy mismatch')
    print(json.dumps(dict(model=m,pk_sha=g.sha(b),recurrent_voice=hex(o),kr_refresh=hex(refresh),
          region=[hex(a),hex(a+n)],ramtext_sha=g.sha(contents),boot_ram_exact=True,
          recoverable_pool_beyond_hard=0x54000-sec['.pool'][1]-8192-512)))
