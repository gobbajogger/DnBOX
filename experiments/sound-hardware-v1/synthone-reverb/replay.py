"""Run the original scheduler, both reverb candidates and the existing lane spectrum/level rule."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import numpy as np
from prepare import prepare


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def compile_probe(build, out):
    cmds = json.loads((build / 'factory-build-commands.json').read_text())
    originals = [next(c for c in cmds if any(a.endswith(name) for a in c))
                 for name in ['/modules/revsc.c','/preset_audio.cpp']]
    for cmd, source in zip(originals, [out / 'compact.c', Path(__file__).with_name('reverb.cpp').resolve()]):
        cmd = cmd.copy()
        cmd[cmd.index('-c')+1] = str(source)
        cmd[cmd.index('-o')+1] = str(out / (source.stem + '.o'))
        cmd[cmd.index('-MF')+1] = str(out / (source.stem + '.d'))
        cmd.insert(1, '-I'+str(out))
        subprocess.run(cmd,check=True)
    cmd = cmds[-1].copy()
    cmd[cmd.index(str(build / 'factory-6.o'))] = str(out / 'reverb.o')
    cmd.insert(1, str(out / 'compact.o'));cmd[cmd.index('-o')+1] = str(out/'probe')
    subprocess.run(cmd,check=True)


def spectrum(x):
    frames = np.lib.stride_tricks.sliding_window_view(x,512)[::128]
    return np.sqrt(np.mean(abs(np.fft.rfft(frames * np.hanning(512),axis=1))**2,axis=0))


def compare(reference, candidate):
    ref, trial = reference.astype(float), candidate.astype(float)
    ref_spectrum, trial_spectrum = spectrum(ref), spectrum(trial)
    floor = max(ref_spectrum.max()*1e-4, 1e-12)
    keep = (ref_spectrum>floor) | (trial_spectrum>floor)
    spectral = np.sqrt(np.mean((20*np.log10(np.maximum(trial_spectrum[keep],floor)/np.maximum(ref_spectrum[keep],floor)))**2)) if keep.any() else 0.
    rms = 10*np.log10(max(1e-20,np.mean(trial*trial)) / max(1e-20,np.mean(ref*ref)))
    return {'signed_rms_db':float(rms),'spectral_rms_db':float(spectral),'peak_error':float(np.abs(trial-ref).max()),'pass':bool(abs(rms)<=1.0 and spectral<=3.0)}


def main():
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument('repo',type=Path);cli.add_argument('factory_build',type=Path);cli.add_argument('output',type=Path)
    args=cli.parse_args();repo,build,out = (p.resolve() for p in (args.repo,args.factory_build,args.output))
    out.mkdir(exist_ok=True,parents=True)
    prepare(repo,out);compile_probe(build,out)
    rows = json.loads((build/'commands/ORIGINAL-PRESET-COMMANDS.json').read_text())['cases']
    data=build/'factory-inputs';result={'source':'AudioKit/AudioKitSynthOne@6466a3715c96b7ecf1dd255a218cbd571408a314','command_sha256':digest(build/'commands/preset-commands.bin'),'sample_rate_hz':44100,'factor':30,'filter_and_dynamics':'Independent original full graphs. Before each reverb call every input word must match across all three graphs. Panning, lowpass/ping-pong, both stereo reverb compressors, master, width and all other source effects/parameters are identical.','contract':{'fft':512,'hop':128,'linear_gain':1,'floor_db':-80,'max_absolute_rms_db':1,'max_spectrum_rms_db':3},'candidates':{'multirate':'Eight mu-law histories at 1470 Hz, block-mean downconversion and causal linear interpolation. Original line times, modulation and feedback.','shortened':'The same eight mu-law histories at 44100 Hz, delays shortened 30x, feedback transformed to its 30th root to compensate decay length.'},'rows':[]}
    for idx in [1,0,8,533,656]:
        path=out/'audio.f32'
        measure=subprocess.run([out/'probe',data/'public-morphs.f32',data/'compact.bin',build/'commands/preset-commands.bin',str(idx),path],check=True,stdout=subprocess.PIPE,text=True)
        words=np.fromfile(path,dtype='<f4').reshape(-1,3,4);assert len(words)==132300 and np.isfinite(words).all()
        row=rows[idx]; c=dict(row['commands']);entry={'program_index':idx,'name':row['name'],'bank':row['bank'],'bank_index':row['preset_index'],'original_reverb_controls':{str(i):c[i] for i in [37,38,39,40,51,123,126,129,132,135,124,127,130,133,136]},'ring_allocation':measure.stdout.strip(),'frames':len(words),'capture_sha256':digest(path),'comparisons':{}}
        for mode,name in enumerate(['multirate','shortened'],1):
            entry['comparisons'][name]={label:compare(words[:,0,k],words[:,mode,k]) for k,label in enumerate(['wet_left','wet_right','final_left','final_right'])}
        result['rows'].append(entry); path.unlink(); print(idx,entry['name'],entry['comparisons'],flush=True)
    (out/'result.json').write_text(json.dumps(result,indent=2)+'\n')

if __name__=='__main__':main()
