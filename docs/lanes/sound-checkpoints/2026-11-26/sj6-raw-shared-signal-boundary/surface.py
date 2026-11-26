from pathlib import Path
import json,tarfile
p=Path('sj6-modern-owner/probe.py')
exec(compile(p.read_text().split('def trial(')[0],str(p),'exec'))
paths.BIN=str(BIN);import objcmeta as o
names=('_TtC10SuperJ6_AU23KingOfDigital_AudioUnit','_TtC10SuperJ6_AU24AKAUv3ExtensionAudioUnit',
       '_TtC8AudioKit7AKMixer')
methods={k:{s:hex(ad) for s,ad in o.classes[k][1]} for k in names}
for k in names[:2]:assert not any(('tap' in s.lower() or 'oscsection' in s.lower() or 'mixer' in s.lower())
                                     for s in methods[k])
assert 'internalRenderBlock' in methods[names[1]] and 'outputBusses' in methods[names[1]]
assert len(o.ivars_any(names[2])) and any(f[0]=='mixerAU' for f in o.ivars_any(names[2]))
p=Path('sj6-cross-preset/SJ6-Three-Factory-Presets-original-AUv3.tgz')
with tarfile.open(p) as tf:
 docs=[n for n in tf.getnames() if n.endswith('.swift') and '._' not in n]
 assert docs
 source='\n'.join(tf.extractfile(n).read().decode() for n in docs)
 assert 'renderOffline' in source and 'parameterTree' in source
 assert not any(s in source for s in ('installTap(', 'oscSection', 'mixerAU')) # directly checked bench source, not guessed API
print(json.dumps(dict(current_binary=pin(BIN),component_interfaces=methods,reachable_readback='terminal outputBusses/render block, parameters and preset state only on this archived interface; no exposed oscSection node selector',
 inner_mixer_field='AKMixer.mixerAU is private graph object state, not provided by bench readback', archived_host_source=docs,
 limitation='selector/source audit of available readback, not a runtime Apple framework reverse-engineer. Existing bench does not offer private mixer access; no use of process memory, mock frames, injection or device.'),indent=2))
