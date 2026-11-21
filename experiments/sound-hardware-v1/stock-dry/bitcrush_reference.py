"""Compare the native INIT rate reducer with the pinned Synth One kernel section."""
import argparse
import hashlib
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
KERNEL = 'closure/master/AudioKitSynthOne/DSP/Kernel/S1DSPKernel+process.mm'
DIGEST = '3fa142e20a4e51869b796743a61e58e04c34e73f81ca6736f5c456a81cadce53'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('restored_source', type=Path)
    args = parser.parse_args()
    source = (args.restored_source / KERNEL).read_bytes()
    if hashlib.sha256(source).hexdigest() != DIGEST:
        raise ValueError('unexpected original kernel')
    text = source.decode()
    section = text[text.index('float bitcrushSrate ='):text.index('bitcrushSampleIndex += 1.f;') + len('bitcrushSampleIndex += 1.f;')]
    # Preserve the upstream float assignments, log/exp round trip and phase.
    # A static control test disconnects all three modulator inputs.
    wrapper = '''#include <math.h>
static float bitcrushIncr, bitcrushIndex, bitcrushSampleIndex, bitcrushValue;
enum {bitCrushSampleRate, bitcrushLFO};
static float sampleRate() { return 44100.f; }
static float clampedValue(int, float f) { return f < 2048.f ? 2048.f : f > 48000.f ? 48000.f : f; }
extern "C" void crush_reference_reset() {
    bitcrushIndex = bitcrushSampleIndex = bitcrushValue = 0.f;
}
extern "C" float crush_reference_sample(float synthOut, float hz) {
    float parameters[] = {hz, 0.f};
    float lfo1_0_1 = 0.f, lfo2_0_1 = 0.f, lfo3_0_1 = 0.f;
''' + section + '\nreturn bitCrushOut;\n}\n'
    dsp = ROOT / 'firmware/src/dsp'
    app = ROOT / 'firmware/src/app'
    with tempfile.TemporaryDirectory() as target:
        work = Path(target)
        (work / 'reference.cpp').write_text(wrapper)
        flags = ['-O2', '-ffp-contract=off', '-ffunction-sections', '-fdata-sections']
        subprocess.run(['c++', *flags, '-c', work / 'reference.cpp', '-o', work / 'original.o'], check=True)
        # The fixture starts with the canonical shipping INIT cell.
        units = [HERE / 'bitcrush_compare.c', app / 's1_dry_part.c', app / 'original_sound.c',
                 dsp / 's1_dry_voice.c', dsp / 'dnbx_s1_envelope.c']
        subprocess.run(['cc', *flags, '-std=c11', '-DX0X_HOST',
                        '-I' + str(app), '-I' + str(dsp), *units, work / 'original.o',
                        '-Wl,--gc-sections', '-lm', '-o', work / 'compare'], check=True)
        subprocess.run([work / 'compare'], check=True)


if __name__ == '__main__':
    main()
