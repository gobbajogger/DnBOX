import json, struct, sys
from pathlib import Path
R, elf, out = map(Path, sys.argv[1:])
sys.path.insert(0,str(R/'restored-harness-inspect/helpers'))
from guarded_replay import ELF_RAM,symbols_and_sections
_,sym,_=symbols_and_sections(elf)
ram=(out/'ram.bin').read_bytes()
meta=json.loads((out/'result.json').read_text())
owners=json.loads((out/'owner.json').read_text())
spans,=struct.unpack_from('<I',ram,sym['lab_joint_fixed_audible_spans']-ELF_RAM)
cue=ram[sym['lab_joint_fixed_cue_seeded']-ELF_RAM]
separate = {k:struct.unpack_from('<I',ram,sym['lab_joint_audible_'+k]-ELF_RAM)[0] for k in ('a','b')}
meta.update(cue=cue,individually_audible_spans=separate,joint_audible_spans=spans,owners=owners,nor_immutable=True,
            audio_gate_qualified=bool(meta['meets_timing'] and cue==1 and spans>0 and owners['allScheduledOwnerReadbacksAgree']))
print(json.dumps(meta))
assert meta['nor_immutable'] and cue==1 and spans>0 and owners['allScheduledOwnerReadbacksAgree'], meta
